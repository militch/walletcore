package com.github.militch.walletj;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

class PlatformUtils {
    private static final String NORMALIZED_ARCH = normalizeArch(SystemPropertyUtil.get("os.arch", ""));
    private static final String NORMALIZED_OS = normalizeOs(SystemPropertyUtil.get("os.name", ""));
    private static final int BIT_MODE = bitMode0();
    private static final boolean IS_WINDOWS = isWindows0();
    private static final boolean IS_OSX = isOsx0();
    private static final File TMPDIR = tmpdir0();

    /**
     * Returns the temporary directory.
     */
    public static File tmpdir() {
        return TMPDIR;
    }
    public static File createTempFile(String prefix, String suffix, File directory) throws IOException {
        if (directory == null) {
            return Files.createTempFile(prefix, suffix).toFile();
        }
        return Files.createTempFile(directory.toPath(), prefix, suffix).toFile();
    }

    @SuppressWarnings("ResultOfMethodCallIgnored")
    private static File toDirectory(String path) {
        if (path == null) {
            return null;
        }

        File f = new File(path);
        f.mkdirs();

        if (!f.isDirectory()) {
            return null;
        }

        try {
            return f.getAbsoluteFile();
        } catch (Exception ignored) {
            return f;
        }
    }
    private static File tmpdir0() {
        File f;
        try {
            f = toDirectory(SystemPropertyUtil.get("java.io.tmpdir"));
            if (f != null) {
                return f;
            }
            // This shouldn't happen, but just in case ..
            if (isWindows()) {
                f = toDirectory(System.getenv("TEMP"));
                if (f != null) {
                    return f;
                }
                String userprofile = System.getenv("USERPROFILE");
                if (userprofile != null) {
                    f = toDirectory(userprofile + "\\AppData\\Local\\Temp");
                    if (f != null) {
                        return f;
                    }

                    f = toDirectory(userprofile + "\\Local Settings\\Temp");
                    if (f != null) {
                        return f;
                    }
                }
            } else {
                f = toDirectory(System.getenv("TMPDIR"));
                if (f != null) {
                    return f;
                }
            }
        } catch (Throwable ignored) {
            // Environment variable inaccessible
        }

        // Last resort.
        if (isWindows()) {
            f = new File("C:\\Windows\\Temp");
        } else {
            f = new File("/tmp");
        }
        return f;
    }
    public static boolean isOsx() {
        return IS_OSX;
    }
    private static int bitMode0() {
        String vm = SystemPropertyUtil.get("java.vm.name", "").toLowerCase(Locale.US);
        Pattern bitPattern = Pattern.compile("([1-9][0-9]+)-?bit");
        Matcher m = bitPattern.matcher(vm);
        if (m.find()) {
            return Integer.parseInt(m.group(1));
        } else {
            return 64;
        }
    }
    public static boolean isWindows() {
        return IS_WINDOWS;
    }
    private static boolean isWindows0() {
        return "windows".equals(NORMALIZED_OS);
    }
    private static String normalize(String value) {
        StringBuilder sb = new StringBuilder(value.length());
        for (int i = 0; i < value.length(); i++) {
            char c = Character.toLowerCase(value.charAt(i));
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
                sb.append(c);
            }
        }
        return sb.toString();
    }
    private static boolean isOsx0() {
        return "osx".equals(NORMALIZED_OS);
    }
    private static String normalizeOs(String value) {
        value = normalize(value);
        if (value.startsWith("aix")) {
            return "aix";
        }
        if (value.startsWith("hpux")) {
            return "hpux";
        }
        if (value.startsWith("os400")) {
            // Avoid the names such as os4000
            if (value.length() <= 5 || !Character.isDigit(value.charAt(5))) {
                return "os400";
            }
        }
        if (value.startsWith("linux")) {
            return "linux";
        }
        if (value.startsWith("macosx") || value.startsWith("osx") || value.startsWith("darwin")) {
            return "osx";
        }
        if (value.startsWith("freebsd")) {
            return "freebsd";
        }
        if (value.startsWith("openbsd")) {
            return "openbsd";
        }
        if (value.startsWith("netbsd")) {
            return "netbsd";
        }
        if (value.startsWith("solaris") || value.startsWith("sunos")) {
            return "sunos";
        }
        if (value.startsWith("windows")) {
            return "windows";
        }
        return "unknown";
    }
    private static String normalizeArch(String value) {
        value = normalize(value);
        switch (value) {
            case "x8664":
            case "amd64":
            case "ia32e":
            case "em64t":
            case "x64":
                return "x86_64";

            case "x8632":
            case "x86":
            case "i386":
            case "i486":
            case "i586":
            case "i686":
            case "ia32":
            case "x32":
                return "x86_32";

            case "ia64":
            case "itanium64":
                return "itanium_64";

            case "sparc":
            case "sparc32":
                return "sparc_32";

            case "sparcv9":
            case "sparc64":
                return "sparc_64";

            case "arm":
            case "arm32":
                return "arm_32";

            case "aarch64":
                return "aarch_64";

            case "riscv64":
                return "riscv64";

            case "ppc":
            case "ppc32":
                return "ppc_32";

            case "ppc64":
                return "ppc_64";

            case "ppc64le":
                return "ppcle_64";

            case "s390":
                return "s390_32";

            case "s390x":
                return "s390_64";

            case "loongarch64":
                return "loongarch_64";

            default:
                return "unknown";
        }
    }
}
