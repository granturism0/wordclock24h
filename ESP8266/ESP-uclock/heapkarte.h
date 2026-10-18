/*----------------------------------------------------------------------------------------------------------------------------------------
 * heapkarte.h - BEFRISTETES MESSINSTRUMENT ZU C80 (specs/paket-2026-10-17-c80, Release I) - DER RUECKBAU GEHOERT ZUR MASSNAHME
 *
 * ZWECK: Eine einzige Anfrage in den ersten Sekunden nach dem Start teilt den groessten freien Block dauerhaft (C80,
 * BEFUNDE.md L499-L501). Die Heap-Zeile sagt, DASS er kleiner ist, nicht WER ihn teilt und WANN. Dieses Instrument
 * zeigt es:
 *   M1 Heap-Karte    "- heap karte <anlass> ..."  Blockgang ueber den inneren Aufbau des Haufens (Ent-C1 = A)
 *   M2 Toepfe        "- heap topf ..."             Belegung je Aufruf in loop () im Startfenster W (60 s ab t0, Ent-C5 = A)
 *   M3 Ereignisse    Karten S, F, X, T             erster Durchlauf, nach der ersten Anfrage, Rueckgang, TIME_WAIT leer
 *   M4 Wegmarken     "- heap marken <X|W> ..."     Blockindex bekannter Objekte, genau einmal je Start
 * Alles nur aus loop (); setup () bleibt unveraendert. In W gibt das Instrument KEINE Zeile aus (jede Ausgabe gibt in
 * Serial.flush () die CPU ab und verfaelschte die Toepfe), danach hoechstens eine Zeile je Sekunde.
 *
 * KOSTEN: "Globale Variablen" +304 Byte (eine Struktur, HK_ZUSTAND, unten per static_assert festgenagelt; Grenze Ent-C2
 * 320). In W 11 Aufrufe von ESP.getFreeHeap () (O(1)) je Durchlauf; je Start hoechstens vier Blockgaenge mit gesperrten
 * Unterbrechungen (gemessen als us=), danach einer je Minute zur Heap-Zeile. Bruecke: je Start 8 bis 11 Zeilen nach W,
 * im Betrieb eine Kartenzeile je Minute. Keine STM-Kommandos (Praefix "- ").
 *
 * BINDUNG: Der Blockgang kennt die Blockform von umm_malloc aus Core 3.1.2. heapkarte.cpp bricht die Uebersetzung mit
 * #error/static_assert ab, sobald ein anderer Core oder eine andere Heap-Konfiguration vorliegt.
 *
 * WEG DAMIT im naechsten ESP-Release nach der Abnahme von Stufe 3 (Ent-C4, AKZ.7): M2-M4 entfernen, M1 entfernen oder
 * ESP_HEAP_MAP auf 0. Mit 0 entfallen alle Zeilen und das BSS; die Makros in loop () werden leer. Steht das Instrument
 * ungeprueft weiter hier, kostet es 304 Byte Haufen - genau die Groesse, um die es in C80 geht.
 *
 * ESP_HEAP_MAP steht HIER und nicht neben ESP_HEAP_LOG in ESP-uclock.ino: heapkarte.cpp und http.cpp sind eigene
 * Uebersetzungseinheiten und saehen ein #define in der .ino nicht. Die Pruefung "ESP_HEAP_MAP 1 braucht ESP_HEAP_LOG 1"
 * steht in ESP-uclock.ino unmittelbar unter ESP_HEAP_LOG.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef HEAPKARTE_H
#define HEAPKARTE_H

#define ESP_HEAP_MAP                1                                       // 0 = aus. Rueckbau nach Stufe 3, siehe oben

#if ESP_HEAP_MAP

#include <stdint.h>
#include <stddef.h>

#define HK_TOPF_WIF                 0                                       // wifi_check_if_started ()
#define HK_TOPF_SRV                 1                                       // http_server_loop ()
#define HK_TOPF_OTA                 2                                       // http_ota_restart_poll ()
#define HK_TOPF_UPD                 3                                       // update_cache_poll ()
#define HK_TOPF_UDP                 4                                       // udp_server_loop ()
#define HK_TOPF_NTP                 5                                       // ntp_poll_time ()
#define HK_TOPF_VAR                 6                                       // var_sync_check ()
#define HK_TOPF_OVR                 7                                       // Block der Ueberlaufabfrage
#define HK_TOPF_HLG                 8                                       // esp_heap_log () samt Ausgabe des Instruments
#define HK_TOPF_STM                 9                                       // stm_bridge_poll ()
#define HK_TOPF_SYS                 10                                      // Ende eines Durchlaufs bis Anfang des naechsten
#define HK_TOEPFE                   11

#define HK_K_M                      0                                       // Karten: m zur Heap-Zeile, dann die Ereignisse
#define HK_K_S                      1
#define HK_K_F                      2
#define HK_K_X                      3
#define HK_K_T                      4
#define HK_KARTEN                   5

#define HK_MARKEN_TCP               4                                       // Plaetze je Liste; was darueber geht, zaehlt +<n>
#define HK_MARKEN_UDP               4

#define HK_W_MS                     60000UL                                 // Startfenster W (Ent-C5 = A)
#define HK_EREIGNIS_MS              180000UL                                // F und T nur bis hier, X nur in W
#define HK_X_ABSTAND                400                                     // X: free < free(S) - 400
#define HK_TAKT_MS                  1000UL                                  // hoechstens eine Zeile je Sekunde

#define HK_NIX                      0xFFFF                                  // Blockindex: Objekt gibt es nicht ("-")
#define HK_FREMD                    0xFFFE                                  // Blockindex: Zeiger ausserhalb des Haufens ("x")

#define HK_M_GESTARTET              0x01                                    // merker: erster Durchlauf erfasst (S)
#define HK_M_W                      0x02                                    // in W: Toepfe messen
#define HK_M_LAEUFT                 0x04                                    // bis 180 s: F, X, T pruefen
#define HK_M_ANGENOMMEN             0x08                                    // F: eine Annahme gesehen
#define HK_M_F                      0x10
#define HK_M_X                      0x20
#define HK_M_T                      0x40
#define HK_M_MARKEN                 0x80

#define HK_O_KOPF                   0x0001                                  // offen: Zeilen in der Reihenfolge der Ausgabe
#define HK_O_TOPF1                  0x0002
#define HK_O_TOPF2                  0x0004
#define HK_O_TOPF3                  0x0008
#define HK_O_TOPF4                  0x0010
#define HK_O_S                      0x0020
#define HK_O_F                      0x0040
#define HK_O_X                      0x0080
#define HK_O_MARKEN                 0x0100
#define HK_O_T                      0x0200
#define HK_O_M                      0x0400
#define HK_O_ALLE                   0x07FF

#define HK_E_S                      0x01                                    // hk_anfang (): jetzt zu erfassen
#define HK_E_F                      0x02
#define HK_E_X                      0x04
#define HK_E_T                      0x08
#define HK_E_MARKEN                 0x10
#define HK_E_M                      0x20

#define HK_AB_BS                    1                                       // abbruch=: Blockgroesse nicht 8
#define HK_AB_NB                    2                                       // Folgeindex >= Blockzahl
#define HK_AB_ORD                   3                                       // Folgeindex nicht groesser als der aktuelle
#define HK_AB_IT                    4                                       // mehr Schritte als Bloecke
#define HK_AB_SUM                   5                                       // Summe aller Eintraege != Blockzahl - 2

typedef struct
{
    int32_t         netto;                                                  // Summe aller Aenderungen in W, Byte
    uint16_t        rueckgang;                                              // groesster einzelner Rueckgang, Byte
    uint16_t        ms;                                                     // ms seit t0 bei diesem Rueckgang
} HK_TOPF;

typedef struct
{
    uint32_t        ms;                                                     // ms seit t0 (S: 0); m traegt es nicht
    uint16_t        belegt;                                                 // Zahl der belegten Eintraege
    uint16_t        frei;                                                   // Zahl der freien Eintraege
    uint16_t        fsum;                                                   // ALLE Laengen in BLOECKEN; ausgegeben x 8
    uint16_t        gross_b;                                                // Bloecke: Index wie im Haufen, HK_NIX = "-"
    uint16_t        gross_n;
    uint16_t        zweit_b;
    uint16_t        zweit_n;
    uint16_t        davor_b;
    uint16_t        davor_n;
    uint16_t        danach_b;
    uint16_t        danach_n;
    uint8_t         abbruch;                                                // 0 oder HK_AB_*
} HK_KARTE;

typedef struct
{
    uint16_t        lst;                                                    // Listen-PCB an Port 80
    uint16_t        wet;                                                    // ClientContext des Wetter-Clients
    uint16_t        dhcp;                                                   // DHCP-Daten der Station
    uint16_t        eep;                                                    // Puffer von EEPROM
    uint16_t        tcp_pcb[HK_MARKEN_TCP];
    uint16_t        tcp_ctx[HK_MARKEN_TCP];
    uint16_t        udp_pcb[HK_MARKEN_UDP];
    uint16_t        udp_ctx[HK_MARKEN_UDP];
    uint8_t         tcp_zst[HK_MARKEN_TCP];                                 // lwIP-Zustand (TIME_WAIT = 10)
    uint8_t         tcp_n;                                                  // Zahl der PCBs, saettigend - auch die ohne Platz
    uint8_t         udp_n;
    char            anlass;                                                 // 'X' oder 'W'
} HK_MARKEN;

typedef struct
{
    HK_KARTE        karte[HK_KARTEN];
    HK_MARKEN       marken;
    HK_TOPF         topf[HK_TOEPFE];
    uint32_t        t0;                                                     // millis () am Anfang des ersten Durchlaufs
    uint32_t        n;                                                      // Durchlaeufe in W
    uint32_t        takt;                                                   // millis () der letzten Zeile (auch Heap-Zeile)
    uint16_t        letzter;                                                // free beim vorigen Lesen (NACH bzw. Anfang)
    uint16_t        free0;                                                  // free am Anfang des ersten Durchlaufs = free(S)
    uint16_t        free1;                                                  // free am Anfang des ersten Durchlaufs nach W
    uint16_t        us;                                                     // laengster Blockgang seit dem Start
    uint16_t        acc_vor;                                                // Annahmezaehler am Anfang des vorigen Durchlaufs
    uint16_t        offen;                                                  // HK_O_*: auszugebende Zeilen
    uint16_t        ms_lauf;                                                // ms seit t0 am Anfang dieses Durchlaufs (in W)
    uint8_t         tw_vor;                                                 // TIME_WAIT-PCBs am Anfang des vorigen Durchlaufs
    uint8_t         merker;                                                 // HK_M_*
} HK_ZUSTAND;

static_assert (sizeof (HK_TOPF) == 8, "HK_TOPF");
static_assert (sizeof (HK_KARTE) == 28, "HK_KARTE");
static_assert (sizeof (HK_MARKEN) == 48, "HK_MARKEN");
static_assert (sizeof (HK_ZUSTAND) == 304, "HK_ZUSTAND: BSS des Instruments, Ent-C2 <= 320 - erst nachrechnen, dann aendern");

extern HK_ZUSTAND           heapkarte;

/* Reine Funktionen (heapkarte.cpp, Abschnitt C80I REIN) - ohne Geraet uebersetzbar, Pruefstand c80i */
extern uint8_t              hk_gang (const void *, uint16_t, uint16_t, HK_KARTE *);
extern uint16_t             hk_index (const void *, const void *, uint16_t);
extern void                 hk_topf_buchen (HK_TOPF *, int32_t, uint16_t);
extern void                 hk_nach (HK_ZUSTAND *, uint8_t, uint16_t);
extern uint8_t              hk_anfang (HK_ZUSTAND *, uint32_t, uint16_t, uint8_t, uint8_t, uint16_t);
extern void                 hk_erfasst (HK_ZUSTAND *, uint8_t, uint32_t);
extern uint8_t              hk_heapzeile (HK_ZUSTAND *, uint32_t);
extern size_t               hk_zeile (const HK_ZUSTAND *, uint8_t, char *, size_t);
extern size_t               hk_ausgabe (HK_ZUSTAND *, uint32_t, char *, size_t);

/* Geraeteteil (heapkarte.cpp) */
extern void                 heapkarte_anfang (void);
extern void                 heapkarte_heapzeile (void);                     // aus esp_heap_log (), vor Serial.println
extern void                 heapkarte_ausgabe (void);
extern uint16_t             http_accept_count_get (void);                   // http.cpp: Annahmezaehler fuer Karte F

/* Messpunkte in loop (). NACH liest free unmittelbar nach einem Topf und bucht ihn gegen letzter - den Wert, den das
 * vorige NACH bzw. der Anfang des Durchlaufs (hk_anfang ()) gelesen hat. Jeder Topf reicht lueckenlos an den naechsten
 * weiter; die Summenprobe ist damit von der Bauart her exakt (11 Lesestellen je Durchlauf: 1 Anfang + 10 NACH).
 * VOR LIEST NICHT (Review 1.7 S1): Ein zweites Lesen ueberschriebe letzter, und eine Belegung aus einer Unterbrechung
 * zwischen NACH und VOR (umm_malloc_cfg.h:303-308, free_blocks_isr_min) fiele in keinen Topf. VOR bleibt als leerer
 * Marker stehen, damit die Struktur in loop () und der Rueckbau gleich bleiben. Wer zwischen NACH und VOR einen Aufruf
 * einfuegt, verfaelscht die Zuordnung: er zaehlt zum naechsten Topf (AKI.4 N2 (e)).
 */
#define HEAPKARTE_ANFANG()          do { if ((heapkarte.merker & (HK_M_GESTARTET | HK_M_W | HK_M_LAEUFT)) != HK_M_GESTARTET) heapkarte_anfang (); } while (0)
#define HEAPKARTE_VOR()             do { } while (0)                       // Marker, liest nicht (S1)
#define HEAPKARTE_NACH(t)           do { if (heapkarte.merker & HK_M_W) hk_nach (&heapkarte, (t), (uint16_t) ESP.getFreeHeap ()); } while (0)
#define HEAPKARTE_AUSGABE()         do { if (heapkarte.offen) heapkarte_ausgabe (); } while (0)

#else

#define HEAPKARTE_ANFANG()          do { } while (0)
#define HEAPKARTE_VOR()             do { } while (0)
#define HEAPKARTE_NACH(t)           do { } while (0)
#define HEAPKARTE_AUSGABE()         do { } while (0)

#endif

#endif
