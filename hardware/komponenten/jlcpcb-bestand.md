# JLCPCB-Bestand der V2-Stückliste für den ESP32-S3-Umbau

Prüfdatum aller Zeilen: **2026-10-17**.

## Quelle und Belegzeichen

- Stückliste: `~/Documents/WordClock-USB C - STM32F411 - V2/production/bom.csv`, 50 Zeilen, 92 Bauteile (nur gelesen).
- Klasse, Lagerbestand, Gehäuse: Teilebibliothek von JLCPCB, abgerufen am 2026-10-17 über die
  Such-Schnittstelle, die auch `jlcpcb.com/parts` verwendet
  (`jlcpcb.com/api/overseas-pcb-order/v1/shoppingCart/smtGood/selectSmtComponentList`, Schlüsselwort = LCSC-Nummer,
  Treffer nur bei exakt gleicher `componentCode`). Alle Zahlen sind damit **●** (Distributor-Datenbank, kein Datenblatt).
  Es ist nicht die gerenderte Webseite, sondern ihre Datenquelle.
- „Basic“ = Feld `componentLibraryType = base`. „Preferred Extended“ = Filter `preferredComponentFlag = true`
  (liefert 1235 Teile); **keine** der Zeilen unten trägt das Flag, alle Nicht-Basic-Zeilen sind also **plain Extended**.
- Stichprobe gegen `lcsc.com` (Schnittstelle `wmsc.lcsc.com`): C17414 gefunden, Gehäuse 0805, gleiche Beschreibung.
  Der Lagerbestand weicht ab (LCSC 45'115'500, JLCPCB 56'527'062). **Massgeblich ist der JLCPCB-Bestand**, weil dort bestückt wird.
- Ob JLCPCB ein Teil per SMT, per THT oder von Hand bestückt, liefert die Quelle **nicht**. „SMD“ unten heisst nur:
  Gehäuse ist ein SMD-Gehäuse. Für THT-Teile steht „THT, Bestückungsweg nicht geprüft“.
- Datenblätter wurden für dieses Blatt **nicht** gelesen (es geht um Bestand, nicht um Eigenschaften). Aussagen über
  elektrische Gleichwertigkeit von Alternativen sind deshalb **○** und nur als Hinweis an den Designer zu lesen.

## Was entfällt, was bleibt

Entfallen (laut Auftrag): U2, U4, X1, C3, C5, C9, C11, C14, L1, R7, R2, R10, R14, R15, R16, R18, SW4, H3, D1, USB1.
**R7 steht nicht in der Stückliste** (R5, R6, R8 sind da, R7 nicht). Die übrigen 19 Referenzen sind enthalten.

- Zeilen der Stückliste: 50. Ganz entfallen: 8 (C3/C5, C9, L1, SW4, U2, U4, USB1, X1). Teilweise entfallen, Zeile bleibt: 6
  (`C1…`, `D1…`, `H3, H6`, `R10…`, `R12…`) → **42 bleibende Zeilen**.
- Bauteile: 92 − 19 = **73** (Probe unten: Summe der Mengen = 73).
- Eine Nachfolge für `USB1` (der 16-polige USB-C-Stecker des Umbaus) steht **nicht** in dieser Stückliste und ist hier
  nicht geprüft; das gehört in das Blatt des USB-C-Steckers.

## Tabelle

Klasse: B = Basic, E = Extended (kein Preferred). Bestand = Stück bei JLCPCB am Prüfdatum.

| Designator-Gruppe | Wert | LCSC | Klasse | Lagerbestand | Prüfdatum | Anmerkung |
|---|---|---|---|---|---|---|
| C1, C12, C15, C16, C17, C19, C21, C24, C25, C29, C4, C6, C7 (13) | 100 nF 50 V X7R 0805 | C49678 | B | 21'260'246 | 2026-10-17 | Yageo CC0805KRX7R9BB104. Gehäuse 0805 passt. **Achtung V3:** C1 trägt im V3-Schaltplan den Wert 1 µF, aber LCSC C49678 (= 100 nF); Wert oder Nummer falsch, Designer klärt (1 µF wäre C28323). |
| C10 | 100 nF 50 V X7R 0805 | C49678 | B | 21'260'007 | 2026-10-17 | Yageo CC0805KRX7R9BB104, **dieselbe Nummer wie C4 u. a.**; Schaltplan-Feld stimmt. **Fehlt in der V2-Stückliste** (Schaltplan: in_bom ja, dnp nein). Bestand bei dieser Abfrage 21'260'007 (Zeile oben 21'260'246 vom früheren Lauf, schwankt). |
| C18 | 1 µF 50 V X7R 0805 | C28323 | B | 2'020'869 | 2026-10-17 | Samsung CL21B105KBFNNNE. |
| C2 | 10 nF 50 V X7R 0805 | C1710 | B | 1'653'076 | 2026-10-17 | Samsung CL21B103KBANNNC. |
| C20, C22 | 10 µF 10 V Tantal TMCP1A106MTRF | C1969735 | E | 33'835 | 2026-10-17 | Vishay, Gehäuse laut Quelle 0805; Footprint `CAP-SMD_L2.0-W1.2` hat dieselben Aussenmasse. Basic-Alternative (○, nur Wert/Gehäuse, Spannungsfestigkeit und Tantal-Eigenschaft nicht verglichen): C15850 (10 µF 25 V X5R 0805, B, 6'588'495) oder C440198 (10 µF 50 V X5R 0805, B, 1'228'503). Der Grund für Tantal in V2 steht nicht in der Stückliste. |
| C23 | 47 µF 10 V X5R 0805 | C124129 | E | 37'104 | 2026-10-17 | Murata GRM21BR61A476ME15L. Basic-Alternativen: C16780 (47 µF **6,3 V** X5R 0805, B, 1'554'181) — Spannungsreserve je nach Schiene knapp, Designer prüft; C96123 (47 µF 10 V X5R **1206**, B, 352'300) — anderer Footprint. |
| C26 | 10 µF 50 V X5R 0805 | C440198 | B | 1'228'503 | 2026-10-17 | Murata GRM21BR61H106KE43L. |
| C27, C28, C8 | 22 µF 25 V X5R 0805 | C45783 | B | 3'808'915 | 2026-10-17 | Samsung CL21A226MAQNNNE. |
| D2, D3 | PMEG2010ER Schottky 20 V 1 A | C82288 | E | 7'366 | 2026-10-17 | Nexperia PMEG2010ER,115. **Gehäuse laut Quelle SOD-123W, Footprint der Stückliste heisst SOD-123** — Passung vor Layout prüfen. Basic-Schottky gibt es nur in SMA: C2480 (SS14) und C8678 (SS34), anderer Footprint, Durchlassspannung nicht verglichen. Einzelheiten im Blatt der Diode. |
| D4, D5 | SMAJ5.0CA, TVS 5 V bidirektional | C10756 | E | 5'318 | 2026-10-17 | Ruilon SMAJ5.0CA, DO-214AC (SMA), Footprint passt. Keine Basic-TVS gefunden (Suche „TVS SMA 5V bidirectional“, „SMAJ5.0“: 0 Treffer). |
| F1 | PTC 1206, 1,5 A, 16 V (SMD1206-150-16) | C46641031 | E | 36'666 | 2026-10-17 | hongjiacheng, 1206. Keine Basic-Sicherung gefunden („resettable fuse“, „PPTC 1206“: 0 Treffer). |
| F2 | PTC 1812, 3 A, 24 V (1812L300/24GR) | C20627123 | E | 45'582 | 2026-10-17 | LUTE, 1812. Keine Basic-Alternative gefunden (wie F1). |
| H6 (H3 entfällt) | Stiftleiste 1x4, Push-Pull, 2,54 mm, PZ254R-11-04P | C492412 | E | 43'984 | 2026-10-17 | XFCN, **THT**, Bestückungsweg nicht geprüft; Quelle nennt „Right Angle“. Menge sinkt von 2 auf 1. Keine Basic-Stiftleiste gefunden („pin header 2.54mm“: 0 Treffer). |
| H4 | Stiftleiste 1x3, PZ254R-11-03P | C492411 | E | 57'418 | 2026-10-17 | wie H6. |
| H5 | Stiftleiste 1x2, PZ254R-11-02P | C492410 | E | 174'668 | 2026-10-17 | wie H6. |
| H7 | Stiftleiste 1x7, PZ254R-11-07P | C492415 | E | 7'093 | 2026-10-17 | wie H6. Stecker zum LED-Board, bleibt unverändert; **niedrigster Bestand der Stiftleisten**. |
| L2 | 4,7 µH 2,6 A (FTC252010S4R7MBCA) | C5832360 | E | 392'763 | 2026-10-17 | cjiang, Gehäuse 1008, Footprint 2520 passt. Keine Basic-Induktivität 4,7 µH gefunden. Einzelheiten im Blatt des Wandlers. |
| LED1 | LED grün 0805, LTST-C171KGKT | C98224 | E | 177'572 | 2026-10-17 | Lite-On, 35 mcd. Keine Basic-Grün-LED gefunden („LED green“, „NCD0805G“: 0 Treffer). |
| LED4, LED5 | LED rot 0805, LTST-C171KRKT | C284871 | E | 76'170 | 2026-10-17 | Lite-On, 54 mcd. Basic-Alternative: C84256 (NationStar NCD0805R1, 0805, rot, 195 mcd, B, 3'887'762) — deutlich heller, Vorwiderstand müsste der Designer prüfen. |
| Q1, Q3 | SI7149DP-T1-GE3, P-FET 30 V 23,7 A | C553982 | E | 10'088 | 2026-10-17 | Vishay, PowerPAK SO-8, Footprint passt. Basic-P-FETs (C15127 AO3401A, C10487 SI2301CDS) sind SOT-23 und strommässig keine Alternative. Siehe Blatt des Schalters. |
| Q2 | MMBF170, N-FET 60 V | C130879 | E | 1'355 | 2026-10-17 | Shikues, SOT-23; **niedriger Bestand**. Basic-Alternative: C8545 (2N7002, SOT-23, B, 2'604'516) — Schwellspannung und Kennwerte nicht verglichen, Designer prüft. |
| R1, R11 | 330 Ω 0805 | C17630 | B | 1'843'239 | 2026-10-17 | Uni-Royal 0805W8F3300T5E. |
| R20, R25, R28, R31, R8, R9 (6) | 100 kΩ 0805 | C149504 | B | 3'671'304 | 2026-10-17 | Uni-Royal 0805W8F1003T5E. R10, R18 entfallen. |
| R12 | 10 kΩ 0805 | C17414 | B | 56'527'062 | 2026-10-17 | Uni-Royal 0805W8F1002T5E. R14, R15, R16, R2 entfallen. |
| R13 | 0 Ω 0805 | C17477 | B | 6'100'743 | 2026-10-17 | Uni-Royal 0805W8F0000T5E. |
| R19 | 82 Ω 0805 | C17841 | E | 72'948 | 2026-10-17 | Uni-Royal 0805W8F820JT5E. **Einziger Widerstandswert, der in 0805 nicht Basic ist** (Basic-Suche „82Ω 0805“: 0 Treffer). Nächster Basic-Wert 100 Ω (C17408, wie R24); ob das tragbar ist, hängt von der Funktion ab und ist nicht geprüft. |
| R21 | 3,3 kΩ 0805 | C26010 | B | 1'923'648 | 2026-10-17 | Uni-Royal 0805W8F3301T5E. |
| R22 | 1 kΩ 0805 (V2) | C17513 | B | 44'401'810 | 2026-10-17 | Uni-Royal 0805W8F1001T5E. Bleibt im Umbau 1 kΩ (Vorgabe des Nutzers vom 17.10.2026: LDR-Lesung bleibt 100 % kompatibel). |
| R23 | 2,2 kΩ 0805 | **C17520** | B | 3'678'940 | 2026-10-17 | Uni-Royal 0805W8F2201T5E, ±1 %. **Fehlt in der V2-Stückliste** (Schaltplan: in_bom ja, dnp nein). Das LCSC-Feld von R23 im Schaltplan trägt **C17841 = 82 Ω (R19), falsch**. Gegenprobe: Suche C4190 liefert 2,2 kΩ nur in 0603. |
| R24 | 100 Ω 0805 | C17408 | B | 8'690'968 | 2026-10-17 | Uni-Royal 0805W8F1000T5E. |
| R26 | 22,1 kΩ 0805 | C204311 | E | 2'095 | 2026-10-17 | Bourns CR0805-FX-2212ELF; **niedrigster Bestand der Widerstände**. Basic-Nachbarwert 22 kΩ (C17560, B, 1'593'461), 0,45 % kleiner. Ob der Wert ein Teiler ist, ist hier nicht geprüft. |
| R27, R30, R6 | 510 Ω 0805 | C17734 | B | 2'502'070 | 2026-10-17 | Uni-Royal 0805W8F5100T5E. |
| R29 | „32 kΩ“ 0805 | C2930156 | E | 56'127 | 2026-10-17 | **Widerspruch in der Stückliste:** Der Wert heisst 32 kΩ, das Teil C2930156 (FOJAN FRC0805F3162TS) ist laut Quelle **31,6 kΩ**. Welcher Wert gewollt ist, entscheidet der Designer. Basic-Nachbarwerte: 30 kΩ C17621 (B, 770'556), 33 kΩ C17633 (B, 403'679), je rund 5 % daneben. |
| R3, R4 | 5,1 kΩ 0805 | C27834 | B | 2'813'033 | 2026-10-17 | Uni-Royal 0805W8F5101T5E. |
| R5 | 4,7 kΩ 0805 | C17673 | B | 4'199'662 | 2026-10-17 | Uni-Royal 0805W8F4701T5E. |
| RN1, RN2 | Widerstandsnetz 4 × 4,7 kΩ, 0402x4 | C29586 | E | 236'564 | 2026-10-17 | Uni-Royal 4D02WGJ0472TCE, Gehäuse passt zum Footprint `RES-ARRAY 0402-8P`. Basic-Alternative: C1980 (4D03WGJ0472T5E, 4 × 4,7 kΩ **0603x4**, B, 1'908'329) — anderer Footprint; oder 4 Einzelwiderstände C17673. |
| SW1, SW2, SW3 | Taster K2-1157SP-I4SW-01 (SMD, rot) | C2909692 | E | 3'938 | 2026-10-17 | Korean Hroparts, SMD, niedriger Bestand. Keine Basic-Taster gefunden (Suche „tactile switch SMD“, „SMD switch 6x6“: 0 Treffer). |
| U1 | DS18B20 (TO-92) | C5451632 | E | 3'465 | 2026-10-17 | Quelle nennt als Hersteller **HXY MOSFET**, nicht Maxim — Nachbau oder Umetikettierung, **nicht geprüft**. TO-92 ist THT, Bestückungsweg nicht geprüft. Einzelheiten im Blatt des Sensors. |
| U10 | SY8301ABC | C2927130 | E | 20'824 | 2026-10-17 | Silergy, SOT-23-6, passt. Einzelheiten im Blatt des Wandlers. |
| U3 | SN74AHCT1G125DBVR | C7484 | E | 51'402 | 2026-10-17 | TI, SOT-23-5, passt. Einzelheiten im Blatt des Pegelwandlers. |
| U5 | DS3231MZ+TRL | C107410 | E | 2'885 | 2026-10-17 | Analog Devices/Maxim, SOIC-8. Einzelheiten im Blatt der Uhr. |
| U6 | AT24C32M/TR | C2688466 | E | 13'120 | 2026-10-17 | HGSEMI (Nachbau des AT24C32), SOP-8, passt. Einzelheiten im Blatt des EEPROMs. |
| U7 | SMTU2032-LF, Batteriehalter CR2032 | C18212226 | E | 629 | 2026-10-17 | Renata. **Niedrigster Bestand der ganzen Liste (629)**. Die Quelle nennt kein Gehäuse („-“): **Passung zum Footprint `BAT-SMD_SMTU2032-LF` nicht geprüft.** Keine Basic-Batteriehalter gefunden („CR2032 holder“: 0 Treffer). |
| U9 | TPS2121RUXR | C485916 | E | 27'942 | 2026-10-17 | TI, VQFN-HR-12 (2x2,5), passt. Einzelheiten im Blatt des Umschalters. |

## Neue Werte für den Umbau

Bis 17.10.2026 stand hier R22 47 kΩ 0805 (C17713); das entfällt, R22 bleibt 1 kΩ (V2-Wert, Zeile oben), weil die LDR-Lesung nach Vorgabe des Nutzers 100 % kompatibel bleiben muss.

| Wert | LCSC | Klasse | Lagerbestand | Prüfdatum | Anmerkung |
|---|---|---|---|---|---|
| C 10 µF 0805 | C15850 (25 V) oder C440198 (50 V, schon als C26 verbaut) | **B** | 6'588'495 / 1'228'503 | 2026-10-17 | Beide X5R ±10 %. Wert fix, Spannungsklasse wählt der Designer. |
| C 1 µF 0805 | C28323 (50 V X7R) | **B** | 2'020'869 | 2026-10-17 | schon als C18 verbaut. |
| C 100 nF 0805 | C49678 (50 V X7R) | **B** | 21'260'246 | 2026-10-17 | schon als C1 u. a. verbaut. |

## Welche 0805-Werte sind Basic

Alle Werte wurden einzeln gegen die Bibliothek geprüft (siehe Tabelle).

- **Basic:** 0 Ω, 100 Ω, 330 Ω, 510 Ω, 1 k, 3,3 k, 4,7 k, 5,1 k, 10 k, 100 k und 2,2 k (C17520, neu geprüft).
  Kondensatoren: 10 nF, 100 nF, 1 µF, 2,2 µF (C9, entfällt), 10 µF (C26), 22 µF.
- **Nicht Basic:** 82 Ω (C17841), 22,1 kΩ (C204311), 31,6/32 kΩ (C2930156), 47 µF 10 V (C124129).
  Für 22,1 k und 31,6 k gibt es keinen Basic-Wert mit gleicher Zahl; für 82 Ω keinen in 0805 (Suche „82Ω 0805“: 0 Treffer).
- 1 MΩ aus der Liste des Auftrags kommt in der Stückliste **nicht vor** und ist **nicht geprüft**. 2,2 kΩ fehlte in der
  V2-Stückliste, steht aber im Schaltplan (R23) und ist seit 17.10.2026 geprüft: **C17520, Basic**.

## Zusammenfassung und Nachzählung

- Zeilen der V2-Stückliste: 50. Nach dem Streichen: 42 Zeilen, 73 Bauteile. **Dazu R23 und C10** (im Schaltplan, nicht in
  der Stückliste): die Tabelle oben hat **44 Zeilen**, Summe der Mengen **75** (Stand 17.10.2026).
- Je Klasse: **Basic 17 Zeilen** (40 Bauteile), **Preferred Extended 0**, **Extended 27 Zeilen**, nicht geprüft 0.
- **Abgleich mit dem V3-Schaltplan** (`hardware/esp32s3/expected.json`, 97 Referenzen; Stücklisten-Sicht in_bom ja, dnp nein: **83**): Alle 75 Bauteile der Tabelle sind in dieser Sicht. **Ohne Zeile hier bleiben 8:** U2, USB1, U11, SW5, R32, C30, C31, C32 (neue Teile, Zeilen in den Komponentenblättern). **Falsche LCSC-Felder im V3-Schaltplan:** R23 trägt C17841 (richtig C17520), C30 (10 µF) trägt C49678 (richtig C15850 oder C440198), C1 (1 µF) trägt C49678 (richtig C28323).
- Alle 42 Zeilen sind **lieferbar** (Bestand > 0, kleinster: U7 629, Q2 1'355, R26 2'095, SW1–3 3'938, U1 3'465).
  Keine LCSC-Nummer ist ungültig; keine Zeile ohne Treffer.
- **Nicht Basic (27 Zeilen):** C20/C22, C23, D2/D3, D4/D5, F1, F2, H6, H4, H5, H7, L2, LED1, LED4/LED5, Q1/Q3, Q2, R19, R26,
  R29, RN1/RN2, SW1–SW3, U1, U10, U3, U5, U6, U7, U9.
- **Basic-Alternative mit Nummer vorhanden** (Passung/Eignung vom Designer zu prüfen): C20/C22 (C15850, C440198), C23 (C16780,
  C96123), D2/D3 (C2480, C8678), LED4/LED5 (C84256), Q2 (C8545), RN1/RN2 (C1980, C17673), R19 (C17408), R26 (C17560),
  R29 (C17621, C17633).
- **Keine Basic-Alternative gefunden:** D4/D5, F1, F2, H4–H7, L2, LED1, Q1/Q3, SW1–SW3, U7 und alle ICs (Einzelquellen).
- **Auffälligkeiten der Stückliste:** R29 trägt 32 kΩ, das Teil ist 31,6 kΩ; D2/D3 Footprint SOD-123, Teil SOD-123W;
  U7 ohne Gehäuseangabe in der Quelle; U1 stammt von „HXY MOSFET“; R7 fehlt in der Stückliste.
- **Was nicht geprüft wurde:** Datenblätter und elektrische Gleichwertigkeit der Alternativen; ob JLCPCB die THT-Teile
  (H4–H7, U1) bestückt; Passung von U7 zum Footprint; 1 MΩ (nicht in der Stückliste); die Nachfolge für USB1.
