# Pruefstand FR (A50, Paket 2026-10-10): zieht den ECHTEN fault_reset() samt der vier Handler aus main.c.
# Aufruf: extract.py <src-Verzeichnis> <ausgabe.c>
# Dazu die statischen Faelle als Makros: kein log_-Aufruf, kein watchdog_reload(), SystemCoreClockUpdate().
import sys, re
src, out = sys.argv[1], sys.argv[2]
t = open(src + '/main.c', 'rb').read().decode('ascii').replace('\r\n', '\n')
s = t.index('\nstatic void\nfault_reset (')
a = t.rfind('#define FAULT_RESET_WAIT_MS', 0, s)
if a < 0 or t.count('#define FAULT_RESET_WAIT_MS') != 1:
    a = s + 1
e = t.index('\n}\n', t.index('\nUsageFault_Handler (void)')) + 3
body = t[s:t.index('\n}\n', s) + 3]                                  # nur fault_reset()
code = re.sub(r'//[^\n]*|/\*.*?\*/', '', body, flags=re.S)           # Kommentare zaehlen nicht
calls_log  = len(re.findall(r'\blog_\w*\s*\(', code))
calls_wd   = len(re.findall(r'\bwatchdog_reload\s*\(', code))
calls_scu  = len(re.findall(r'\bSystemCoreClockUpdate\s*\(', code))
calls_rst  = len(re.findall(r'\bNVIC_SystemReset\s*\(', code))
handlers   = len(re.findall(r'\n(HardFault|MemManage|BusFault|UsageFault)_Handler \(void\)\n\{\n    fault_reset \("\1"\);\n\}', t[a:e]))
open(out, 'w').write(
    "#define FR_LOG_CALLS %d\n#define FR_WD_CALLS %d\n#define FR_SCU_CALLS %d\n#define FR_RST_CALLS %d\n#define FR_HANDLERS %d\n"
    % (calls_log, calls_wd, calls_scu, calls_rst, handlers)
    + "#line %d \"%s/main.c\"\n" % (t[:a].count('\n') + 1, src) + t[a:e])
print("extrahiert aus", src, ":", t[a:e].count('\n'), "Zeilen; log_", calls_log, "watchdog_reload", calls_wd,
      "SystemCoreClockUpdate", calls_scu, "NVIC_SystemReset", calls_rst, "Handler", handlers)
