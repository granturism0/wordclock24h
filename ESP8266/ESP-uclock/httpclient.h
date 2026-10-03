/*----------------------------------------------------------------------------------------------------------------------------------------
 * httpclient.h - http client
 *
 * Copyright (c) 2018-2025 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

/* Lesefehler-Vertrag (BEFUNDE.md L152):
 * httpclient_read ()      liefert < 0 bei Zeitgrenze oder Abbruch der Gegenstelle
 *                         und laesst *lenp dabei UNVERAENDERT. Jede Schleife der
 *                         Form "while (len > 0)" muss deshalb bei < 0 abbrechen.
 * httpclient_read_line () liefert aus demselben Grund -1 statt einer Zeilenlaenge.
 * Ein unveraendertes *lenp > 0 nach dem Abruf heisst: unvollstaendig geladen.
 */
extern int    httpclient (const char *, const char *, const char *);
extern int    httpclient_read (int *);
extern int    httpclient_read_line (unsigned char *, int, int *);
extern void   httpclient_stop (void);

#endif
