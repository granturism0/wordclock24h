/*----------------------------------------------------------------------------------------------------------------------------------------
 * sw.js - WordClock progressive web app service worker
 *
 * Copyright (c) 2026 Daniel Kocher - danny(at)ewanet.ch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
const CACHE_NAME = "wordclock-app-v86";
const ASSETS = [
  "/app/",
  "/app/index.html",
  "/app/styles.css",
  "/app/app.js",
  "/app/layout-previews.json",
  "/app/manifest.webmanifest",
  "/app/icons/icon-192.svg",
  "/app/icons/icon-512.svg",
  "/app/icons/icon-192.png",
  "/app/icons/icon-512.png",
  "/app/icons/icon-180.png",
  "/app/icons/icon-mask.png"
];

/* Massnahme 7, PWA-Seite.
 *
 * response.ok ist bei einer Antwort der Laenge 0 TRUE. Genau deshalb hat der frueher
 * hier stehende Pfad eine leere Datei anstandslos in den Cache gelegt -- und dort
 * ueberlebt sie jedes Neuladen, weil der Service Worker sie ab dann selbst
 * ausliefert. Eine leere app.js.gz ergibt einen weissen Bildschirm, und der Weg
 * heraus fuehrt nur noch ueber das Loeschen der Browserdaten. Das ist im Projekt
 * bereits passiert.
 *
 * Gemessen wird zuerst an Content-Length: Die Auslieferung der .gz-Assets setzt den
 * Kopf, und das kostet nichts. Fehlt er, wird eine Kopie des Rumpfes vermessen.
 */
async function responseHasContent(response) {
  if (!response || !response.ok) {
    return false;
  }

  const declaredLength = response.headers.get("content-length");

  if (declaredLength !== null && declaredLength !== "") {
    return Number(declaredLength) > 0;
  }

  try {
    const buffer = await response.clone().arrayBuffer();
    return buffer.byteLength > 0;
  } catch (_) {
    return false;
  }
}

/* Nimmt bereits einen Klon entgegen. Ist er leer, wird der Eintrag nicht nur nicht
 * geschrieben, sondern ein vorhandener ALTER Eintrag geloescht: Sonst bliebe eine
 * Fassung im Cache stehen, von der niemand mehr weiss, zu welchem Stand sie gehoert.
 */
async function cacheIfNotEmpty(cache, request, candidate) {
  if (!(await responseHasContent(candidate))) {
    await cache.delete(request);
    return;
  }

  await cache.put(request, candidate);
}

/* Ersetzt cache.addAll(): addAll prueft ebenfalls nur den Status und legt eine
 * 0-Byte-Datei ohne Murren ab. Schlaegt es hier fehl, scheitert die Installation --
 * gewollt. Der bisherige Service Worker bleibt dann aktiv, und das ist in jedem Fall
 * besser als ein Cache mit einer leeren Datei darin.
 */
async function precacheAssets() {
  const cache = await caches.open(CACHE_NAME);
  const emptyAssets = [];

  for (const asset of ASSETS) {
    const response = await fetch(asset, { cache: "reload" });

    if (!response || !response.ok) {
      throw new Error("App-Asset nicht ladbar: " + asset);
    }

    const candidate = response.clone();

    if (!(await responseHasContent(response))) {
      emptyAssets.push(asset);
      continue;
    }

    await cache.put(asset, candidate);
  }

  if (emptyAssets.length) {
    throw new Error("Leere App-Assets, Installation abgebrochen: " + emptyAssets.join(", "));
  }
}

self.addEventListener("install", (event) => {
  event.waitUntil(precacheAssets().then(() => self.skipWaiting()));
});

self.addEventListener("activate", (event) => {
  event.waitUntil(
    caches.keys().then((keys) => Promise.all(keys.filter((key) => key !== CACHE_NAME).map((key) => caches.delete(key))))
      .then(() => self.clients.claim())
  );
});

self.addEventListener("message", (event) => {
  if (event.data && event.data.type === "SKIP_WAITING") {
    self.skipWaiting();
  }
});

self.addEventListener("fetch", (event) => {
  if (event.request.method !== "GET") {
    return;
  }

  const requestUrl = new URL(event.request.url);

  if (requestUrl.origin !== self.location.origin || !requestUrl.pathname.startsWith("/app/")) {
    return;
  }

  if (requestUrl.pathname === "/app/" || requestUrl.pathname === "/app/index.html" || event.request.mode === "navigate") {
    event.respondWith(networkFirst(event.request));
    return;
  }

  event.respondWith(staleWhileRevalidate(event.request));
});

async function networkFirst(request) {
  const cache = await caches.open(CACHE_NAME);

  try {
    const response = await fetch(request);

    if (response && response.ok) {
      // Klon SOFORT ziehen, die Laengenpruefung laeuft danach nebenher: Die Seite soll
      // auf die Vermessung nicht warten muessen.
      void cacheIfNotEmpty(cache, request, response.clone());
    }

    return response;
  } catch (_) {
    const cached = await matchNonEmpty(cache, request);
    if (cached) {
      return cached;
    }
    throw _;
  }
}

/* Auch beim LESEN geprueft, nicht nur beim Schreiben. Ein Cache, der vor dieser
 * Fassung angelegt wurde, kann eine 0-Byte-Datei enthalten -- und ohne diese Pruefung
 * liefert der Service Worker sie bis in alle Ewigkeit aus, auch wenn das Geraet die
 * Datei laengst wieder vollstaendig hergibt. Der leere Eintrag wird dabei entfernt.
 */
async function matchNonEmpty(cache, request) {
  const cached = await cache.match(request);

  if (!cached) {
    return null;
  }

  if (await responseHasContent(cached)) {
    return cached;
  }

  await cache.delete(request);
  return null;
}

async function staleWhileRevalidate(request) {
  const cache = await caches.open(CACHE_NAME);
  const cached = await matchNonEmpty(cache, request);

  const networkPromise = fetch(request)
    .then((response) => {
      if (response && response.ok) {
        void cacheIfNotEmpty(cache, request, response.clone());
      }
      return response;
    })
    .catch((error) => {
      // Ohne brauchbaren Cache-Eintrag darf der Fehler nicht zu "undefined" werden:
      // event.respondWith(undefined) ist selbst ein Fehler und verdeckt den echten.
      if (cached) {
        return cached;
      }
      throw error;
    });

  return cached || networkPromise;
}
