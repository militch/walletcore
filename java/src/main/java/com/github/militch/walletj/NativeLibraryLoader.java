package com.github.militch.walletj;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.attribute.PosixFilePermission;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.EnumSet;
import java.util.Enumeration;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ThreadLocalRandom;

/**
 * Helper class to load JNI resources.
 */
final class NativeLibraryLoader {
    private static final String NATIVE_RESOURCE_HOME = "META-INF/native/";
    private static final File WORKDIR = PlatformUtils.tmpdir();

    // Just use a-Z and numbers as valid ID bytes.
    private static final byte[] UNIQUE_ID_BYTES =
            "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ".getBytes(StandardCharsets.US_ASCII);


    /**
     * Calculates the mangled shading prefix added to this class's full name.
     *
     * <p>This method mangles the package name as follows, so we can unmangle it back later:
     * <ul>
     *   <li>{@code _} to {@code _1}</li>
     *   <li>{@code .} to {@code _}</li>
     * </ul>
     *
     * <p>Note that we don't mangle non-ASCII characters here because it's extremely unlikely to have
     * a non-ASCII character in a package name. For more information, see:
     * <ul>
     *   <li><a href="https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/design.html">JNI
     *       specification</a></li>
     *   <li>{@code parsePackagePrefix()} in {@code netty_jni_util.c}.</li>
     * </ul>
     *
     * @throws UnsatisfiedLinkError if the shader used something other than a prefix
     */
    private static String calculateMangledPackagePrefix() {
        String maybeShaded = NativeLibraryLoader.class.getName();
        // Use ! instead of . to avoid shading utilities from modifying the string
        String expected = "com!github!militch!walletj!NativeLibraryLoader".replace('!', '.');
        if (!maybeShaded.endsWith(expected)) {
            throw new UnsatisfiedLinkError(String.format(
                    "Could not find prefix added to %s to get %s. When shading, only adding a "
                            + "package prefix is supported", expected, maybeShaded));
        }
        return maybeShaded.substring(0, maybeShaded.length() - expected.length())
                .replace("_", "_1")
                .replace('.', '_');
    }

    /**
     * Load the given library with the specified {@link ClassLoader}
     */
    public static void load(String originalName, ClassLoader loader) {
        String mangledPackagePrefix = calculateMangledPackagePrefix();
        String name = mangledPackagePrefix + originalName;
        List<Throwable> suppressed = new ArrayList<>();
        try {
            // first try to load from java.library.path
            NativeLibraryUtil.loadLibrary(name, false);
            return;
        } catch (Throwable ex) {
            suppressed.add(ex);
        }

        String libname = System.mapLibraryName(name);
        String path = NATIVE_RESOURCE_HOME + libname;

        File tmpFile = null;
        URL url = getResource(path, loader);
        try {
            if (url == null) {
                if (PlatformUtils.isOsx()) {
                    String fileName = path.endsWith(".jnilib") ? NATIVE_RESOURCE_HOME + "lib" + name + ".dynlib" :
                            NATIVE_RESOURCE_HOME + "lib" + name + ".jnilib";
                    url = getResource(fileName, loader);
                    if (url == null) {
                        FileNotFoundException fnf = new FileNotFoundException(fileName);
                        ThrowableUtil.addSuppressedAndClear(fnf, suppressed);
                        throw fnf;
                    }
                } else {
                    FileNotFoundException fnf = new FileNotFoundException(path);
                    ThrowableUtil.addSuppressedAndClear(fnf, suppressed);
                    throw fnf;
                }
            }

            int index = libname.lastIndexOf('.');
            String prefix = libname.substring(0, index);
            String suffix = libname.substring(index);

            tmpFile = PlatformUtils.createTempFile(prefix, suffix, WORKDIR);
            try (InputStream in = url.openStream();
                 OutputStream out = Files.newOutputStream(tmpFile.toPath())) {

                byte[] buffer = new byte[8192];
                int length;
                while ((length = in.read(buffer)) > 0) {
                    out.write(buffer, 0, length);
                }
                out.flush();

                if (shouldShadedLibraryIdBePatched(mangledPackagePrefix)) {
                    // Let's try to patch the id and re-sign it. This is a best-effort and might fail if a
                    // SecurityManager is setup or the right executables are not installed :/
                    tryPatchShadedLibraryIdAndSign(tmpFile, originalName);
                }
            }
            // Close the output stream before loading the unpacked library,
            // because otherwise Windows will refuse to load it when it's in use by other process.
            NativeLibraryUtil.loadLibrary(tmpFile.getPath(), true);

        } catch (UnsatisfiedLinkError e) {
            try {
                if (tmpFile != null && tmpFile.isFile() && tmpFile.canRead() &&
                        !NoexecVolumeDetector.canExecuteExecutable(tmpFile)) {
                    // Pass "io.netty.native.workdir" as an argument to allow shading tools to see
                    // the string. Since this is printed out to users to tell them what to do next,
                    // we want the value to be correct even when shading.
                    String message = String.format(
                            "%s exists but cannot be executed even when execute permissions set; " +
                                    "check volume for \"noexec\" flag; use -D%s=[path] " +
                                    "to set native working directory separately.",
                            tmpFile.getPath(), "io.netty.native.workdir");
                    suppressed.add(ThrowableUtil.unknownStackTrace(
                            new UnsatisfiedLinkError(message), NativeLibraryLoader.class, "load"));
                }
            } catch (Throwable t) {
                suppressed.add(t);
            }
            // Re-throw to fail the load
            ThrowableUtil.addSuppressedAndClear(e, suppressed);
            throw e;
        } catch (Exception e) {
            UnsatisfiedLinkError ule = new UnsatisfiedLinkError("could not load a native library: " + name);
            ule.initCause(e);
            ThrowableUtil.addSuppressedAndClear(ule, suppressed);
            throw ule;
        } finally {
            // After we load the library it is safe to delete the file.
            // We delete the file immediately to free up resources as soon as possible,
            // and if this fails fallback to deleting on JVM exit.
            if (tmpFile != null && !tmpFile.delete()) {
                tmpFile.deleteOnExit();
            }
        }
    }

    private static URL getResource(String path, ClassLoader loader) {
        final Enumeration<URL> urls;
        try {
            if (loader == null) {
                urls = ClassLoader.getSystemResources(path);
            } else {
                urls = loader.getResources(path);
            }
        } catch (IOException iox) {
            throw new RuntimeException("An error occurred while getting the resources for " + path, iox);
        }

        List<URL> urlsList = Collections.list(urls);
        int size = urlsList.size();
        switch (size) {
            case 0:
                return null;
            case 1:
                return urlsList.get(0);
            default:

                try {
                    MessageDigest md = MessageDigest.getInstance("SHA-256");
                    // We found more than 1 resource with the same name. Let's check if the content of the file is
                    // the same as in this case it will not have any bad effect.
                    URL url = urlsList.get(0);
                    byte[] digest = digest(md, url);
                    boolean allSame = true;
                    if (digest != null) {
                        for (int i = 1; i < size; i++) {
                            byte[] digest2 = digest(md, urlsList.get(i));
                            if (digest2 == null || !Arrays.equals(digest, digest2)) {
                                allSame = false;
                                break;
                            }
                        }
                    } else {
                        allSame = false;
                    }
                    if (allSame) {
                        return url;
                    }
                } catch (NoSuchAlgorithmException ignored) {
                }
                throw new IllegalStateException(
                        "Multiple resources found for '" + path + "' with different content: " + urlsList);
        }
    }

    private static byte[] digest(MessageDigest digest, URL url) {
        try (InputStream in = url.openStream()) {
            byte[] bytes = new byte[8192];
            int i;
            while ((i = in.read(bytes)) != -1) {
                digest.update(bytes, 0, i);
            }
            return digest.digest();
        } catch (IOException e) {
            return null;
        }
    }

    static void tryPatchShadedLibraryIdAndSign(File libraryFile, String originalName) {
        if (!new File("/Library/Developer/CommandLineTools").exists()) {
            return;
        }
        String newId = new String(generateUniqueId(originalName.length()), StandardCharsets.US_ASCII);
        if (!tryExec("install_name_tool -id " + newId + " " + libraryFile.getAbsolutePath())) {
            return;
        }

        tryExec("codesign -s - " + libraryFile.getAbsolutePath());
    }

    private static boolean tryExec(String cmd) {
        try {
            int exitValue = Runtime.getRuntime().exec(cmd).waitFor();
            return exitValue == 0;
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        } catch (IOException | SecurityException ignored) {
        }
        return false;
    }

    private static boolean shouldShadedLibraryIdBePatched(String packagePrefix) {
        return PlatformUtils.isOsx() && !packagePrefix.isEmpty();
    }

    private static byte[] generateUniqueId(int length) {
        byte[] idBytes = new byte[length];
        for (int i = 0; i < idBytes.length; i++) {
            // We should only use bytes as replacement that are in our UNIQUE_ID_BYTES array.
            idBytes[i] = UNIQUE_ID_BYTES[ThreadLocalRandom.current()
                    .nextInt(UNIQUE_ID_BYTES.length)];
        }
        return idBytes;
    }

    private NativeLibraryLoader() {
        // Utility
    }

    private static final class NoexecVolumeDetector {

        private static boolean canExecuteExecutable(File file) throws IOException {
            // If we can already execute, there is nothing to do.
            if (file.canExecute()) {
                return true;
            }

            // On volumes, with noexec set, even files with the executable POSIX permissions will fail to execute.
            // The File#canExecute() method honors this behavior, probaby via parsing the noexec flag when initializing
            // the UnixFileStore, though the flag is not exposed via a public API.  To find out if library is being
            // loaded off a volume with noexec, confirm or add executalbe permissions, then check File#canExecute().
            Set<PosixFilePermission> existingFilePermissions = Files.getPosixFilePermissions(file.toPath());
            Set<PosixFilePermission> executePermissions =
                    EnumSet.of(PosixFilePermission.OWNER_EXECUTE,
                            PosixFilePermission.GROUP_EXECUTE,
                            PosixFilePermission.OTHERS_EXECUTE);
            if (existingFilePermissions.containsAll(executePermissions)) {
                return false;
            }

            Set<PosixFilePermission> newPermissions = EnumSet.copyOf(existingFilePermissions);
            newPermissions.addAll(executePermissions);
            Files.setPosixFilePermissions(file.toPath(), newPermissions);
            return file.canExecute();
        }

        private NoexecVolumeDetector() {
            // Utility
        }
    }
}
