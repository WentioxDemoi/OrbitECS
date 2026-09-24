import csv
import io
import os
import time
import requests


# ============================================================
# CONFIGURATION
# ============================================================

EPOCH = "2026-09-08 12:00:00"

CENTER = "500@0"

OUT_UNITS = "KM-S"
REF_SYSTEM = "ICRF"
REF_PLANE = "FRAME"

HORIZONS_API_URL = (
    "https://ssd.jpl.nasa.gov/api/horizons.api"
)

SBDB_QUERY_URL = (
    "https://ssd-api.jpl.nasa.gov/sbdb_query.api"
)

G_KM3_KG_S2 = 6.67430e-20


# ============================================================
# NOMBRE D'ASTÉROÏDES
# ============================================================

MAX_ASTEROIDS = 100_000

# Nombre d'objets récupérés par requête SBDB
SBDB_PAGE_SIZE = 1000

# Pause entre requêtes Horizons
REQUEST_DELAY = 0

# Nombre maximum de tentatives
MAX_RETRIES = 5


# ============================================================
# FICHIERS
# ============================================================

METADATA_FILE = (
    "asteroids_metadata.csv"
)

OUTPUT_FILE = (
    "asteroids_initial_state.csv"
)

PROGRESS_FILE = (
    "asteroids_progress.txt"
)


# ============================================================
# SESSION HTTP
# ============================================================

SESSION = requests.Session()

SESSION.headers.update({
    "User-Agent":
        "SolarSystemNBody/1.0 "
        "(JPL Horizons data retrieval)"
})


# ============================================================
# SBDB
# ============================================================

def fetch_sbdb_page(
    limit_from,
    limit
):

    params = {

        "fields": (
            "spkid,"
            "full_name,"
            "pdes,"
            "name,"
            "kind,"
            "neo,"
            "pha,"
            "class,"
            "H,"
            "diameter,"
            "GM"
        ),

        # Astéroïdes uniquement
        "sb-kind": "a",

        "limit": str(limit),

        "limit-from": str(limit_from),
    }

    response = SESSION.get(
        SBDB_QUERY_URL,
        params=params,
        timeout=60,
    )

    response.raise_for_status()

    data = response.json()

    if "data" not in data:

        raise RuntimeError(
            "Réponse SBDB invalide:\n"
            f"{data}"
        )

    fields = data["fields"]

    rows = data["data"]

    index = {
        name: i
        for i, name in enumerate(fields)
    }

    result = []

    for row in rows:

        def get(field):

            i = index.get(field)

            if i is None:
                return None

            return row[i]

        result.append({

            "spkid":
                get("spkid"),

            "full_name":
                get("full_name"),

            "pdes":
                get("pdes"),

            "name":
                get("name"),

            "kind":
                get("kind"),

            "neo":
                get("neo"),

            "pha":
                get("pha"),

            "class":
                get("class"),

            "H":
                get("H"),

            "diameter_km":
                get("diameter"),

            "gm_km3_s2":
                get("GM"),
        })

    return result


# ============================================================
# RÉCUPÉRATION DE 100 000 ASTÉROÏDES
# ============================================================

def build_metadata():

    print()
    print(
        "================================================"
    )
    print(
        "SBDB - récupération des astéroïdes"
    )
    print(
        "================================================"
    )
    print()

    if os.path.exists(METADATA_FILE):

        print(
            f"{METADATA_FILE} existe déjà."
        )

        print(
            "On le réutilise."
        )

        return

    fieldnames = [
        "spkid",
        "full_name",
        "pdes",
        "name",
        "kind",
        "neo",
        "pha",
        "class",
        "H",
        "diameter_km",
        "gm_km3_s2",
    ]

    total = 0

    with open(
        METADATA_FILE,
        "w",
        newline="",
        encoding="utf-8",
    ) as f:

        writer = csv.DictWriter(
            f,
            fieldnames=fieldnames,
        )

        writer.writeheader()

        offset = 0

        while total < MAX_ASTEROIDS:

            remaining = (
                MAX_ASTEROIDS - total
            )

            page_size = min(
                SBDB_PAGE_SIZE,
                remaining
            )

            print(
                f"SBDB: "
                f"{total:,}/{MAX_ASTEROIDS:,}"
            )

            rows = fetch_sbdb_page(
                offset,
                page_size
            )

            if not rows:
                break

            for row in rows:

                writer.writerow(row)

            total += len(rows)

            offset += len(rows)

            if len(rows) < page_size:

                break

    print()

    print(
        f"Metadata sauvegardées : {total:,}"
    )

    print(
        f"Fichier : {METADATA_FILE}"
    )


# ============================================================
# PROGRESSION
# ============================================================

def load_progress():

    if not os.path.exists(
        PROGRESS_FILE
    ):

        return 0

    with open(
        PROGRESS_FILE,
        "r",
        encoding="utf-8",
    ) as f:

        text = f.read().strip()

    if not text:
        return 0

    return int(text)


def save_progress(index):

    temporary = (
        PROGRESS_FILE + ".tmp"
    )

    with open(
        temporary,
        "w",
        encoding="utf-8",
    ) as f:

        f.write(str(index))

    os.replace(
        temporary,
        PROGRESS_FILE
    )


# ============================================================
# INITIALISATION DU CSV FINAL
# ============================================================

def create_output():

    if os.path.exists(
        OUTPUT_FILE
    ):

        return

    fieldnames = [

        "name",
        "designation",
        "full_name",
        "spkid",

        "epoch",

        "x_km",
        "y_km",
        "z_km",

        "vx_km_s",
        "vy_km_s",
        "vz_km_s",

        "gm_km3_s2",
        "mass_kg",

        "diameter_km",
        "radius_km",

        "H",
        "neo",
        "pha",
        "orbit_class",
    ]

    with open(
        OUTPUT_FILE,
        "w",
        newline="",
        encoding="utf-8",
    ) as f:

        writer = csv.DictWriter(
            f,
            fieldnames=fieldnames,
        )

        writer.writeheader()


# ============================================================
# HORIZONS
# ============================================================

def fetch_horizons(
    asteroid
):

    spkid = asteroid.get(
        "spkid"
    )

    if not spkid:

        raise RuntimeError(
            "SPK-ID absent."
        )

    # --------------------------------------------------------
    # IMPORTANT :
    #
    # Pour un SPK-ID, Horizons attend :
    #
    # DES=2000001;
    #
    # et PAS simplement :
    #
    # COMMAND=2000001
    #
    # --------------------------------------------------------

    command = (
        f"DES={spkid};"
    )

    params = {

        "format": "text",

        "COMMAND":
            f"'{command}'",

        "OBJ_DATA": "NO",

        "MAKE_EPHEM": "YES",

        "EPHEM_TYPE":
            "VECTORS",

        "CENTER":
            f"'{CENTER}'",

        "TLIST":
            f"'{EPOCH}'",

        "TIME_TYPE":
            "TDB",

        "REF_SYSTEM":
            REF_SYSTEM,

        "REF_PLANE":
            REF_PLANE,

        "OUT_UNITS":
            OUT_UNITS,

        "VEC_TABLE":
            "2",

        "VEC_CORR":
            "NONE",

        "VEC_LABELS":
            "YES",

        "CSV_FORMAT":
            "YES",
    }

    last_error = None

    for attempt in range(
        1,
        MAX_RETRIES + 1
    ):

        try:

            response = SESSION.get(
                HORIZONS_API_URL,
                params=params,
                timeout=60,
            )

            response.raise_for_status()

            text = response.text

            if (
                "$$SOE" not in text
                or
                "$$EOE" not in text
            ):

                raise RuntimeError(
                    text
                )

            data = (
                text
                .split(
                    "$$SOE",
                    1
                )[1]
                .split(
                    "$$EOE",
                    1
                )[0]
                .strip()
            )

            lines = (
                data.splitlines()
            )

            if not lines:

                raise RuntimeError(
                    "Réponse Horizons vide."
                )

            # ------------------------------------------------
            # CSV_FORMAT=YES
            # ------------------------------------------------

            line = lines[0]

            row = next(
                csv.reader(
                    io.StringIO(line)
                )
            )

            if len(row) < 8:

                raise RuntimeError(
                    "Vecteur Horizons incomplet:\n"
                    + line
                )

            # ------------------------------------------------
            # GM
            # ------------------------------------------------

            gm = (
                asteroid.get(
                    "gm_km3_s2"
                )
            )

            try:

                gm = float(gm)

            except (
                TypeError,
                ValueError
            ):

                gm = None

            # ------------------------------------------------
            # Masse
            # ------------------------------------------------

            if gm is not None:

                mass_kg = (
                    gm /
                    G_KM3_KG_S2
                )

            else:

                mass_kg = None

            # ------------------------------------------------
            # Diamètre
            # ------------------------------------------------

            diameter = (
                asteroid.get(
                    "diameter_km"
                )
            )

            try:

                diameter = float(
                    diameter
                )

            except (
                TypeError,
                ValueError
            ):

                diameter = None

            if diameter is not None:

                radius = (
                    diameter / 2.0
                )

            else:

                radius = None

            # ------------------------------------------------
            # Résultat
            # ------------------------------------------------

            return {

                "name":
                    asteroid.get(
                        "name"
                    ),

                "designation":
                    asteroid.get(
                        "pdes"
                    ),

                "full_name":
                    asteroid.get(
                        "full_name"
                    ),

                "spkid":
                    asteroid.get(
                        "spkid"
                    ),

                "epoch":
                    EPOCH,

                "x_km":
                    float(row[2]),

                "y_km":
                    float(row[3]),

                "z_km":
                    float(row[4]),

                "vx_km_s":
                    float(row[5]),

                "vy_km_s":
                    float(row[6]),

                "vz_km_s":
                    float(row[7]),

                "gm_km3_s2":
                    gm,

                "mass_kg":
                    mass_kg,

                "diameter_km":
                    diameter,

                "radius_km":
                    radius,

                "H":
                    asteroid.get(
                        "H"
                    ),

                "neo":
                    asteroid.get(
                        "neo"
                    ),

                "pha":
                    asteroid.get(
                        "pha"
                    ),

                "orbit_class":
                    asteroid.get(
                        "class"
                    ),
            }

        except Exception as e:

            last_error = e

            wait = (
                2 ** (attempt - 1)
            )

            print(
                f" retry {attempt}/"
                f"{MAX_RETRIES}",
                end=""
            )

            time.sleep(wait)

    raise RuntimeError(
        str(last_error)
    )


# ============================================================
# TRAITEMENT PRINCIPAL
# ============================================================

def process_asteroids():

    print()
    print(
        "================================================"
    )
    print(
        "HORIZONS - états initiaux"
    )
    print(
        "================================================"
    )
    print()

    create_output()

    # --------------------------------------------------------
    # Charger metadata
    # --------------------------------------------------------

    with open(
        METADATA_FILE,
        "r",
        encoding="utf-8",
    ) as f:

        asteroids = list(
            csv.DictReader(f)
        )

    total = len(
        asteroids
    )

    print(
        f"Astéroïdes à traiter : "
        f"{total:,}"
    )

    # --------------------------------------------------------
    # Reprendre où on s'était arrêté
    # --------------------------------------------------------

    start_index = load_progress()

    print(
        f"Reprise à l'objet : "
        f"{start_index:,}"
    )

    print()

    # --------------------------------------------------------
    # Append
    # --------------------------------------------------------

    fieldnames = [

        "name",
        "designation",
        "full_name",
        "spkid",

        "epoch",

        "x_km",
        "y_km",
        "z_km",

        "vx_km_s",
        "vy_km_s",
        "vz_km_s",

        "gm_km3_s2",
        "mass_kg",

        "diameter_km",
        "radius_km",

        "H",
        "neo",
        "pha",
        "orbit_class",
    ]

    with open(
        OUTPUT_FILE,
        "a",
        newline="",
        encoding="utf-8",
    ) as f:

        writer = csv.DictWriter(
            f,
            fieldnames=fieldnames,
        )

        for index in range(
            start_index,
            total
        ):

            asteroid = (
                asteroids[index]
            )

            display_name = (
                asteroid.get(
                    "name"
                )
                or
                asteroid.get(
                    "full_name"
                )
                or
                asteroid.get(
                    "pdes"
                )
                or
                asteroid.get(
                    "spkid"
                )
            )

            print(
                f"[{index + 1:,}/"
                f"{total:,}] "
                f"{display_name} ...",
                end=" "
            )

            try:

                result = fetch_horizons(
                    asteroid
                )

                writer.writerow(
                    result
                )

                f.flush()

                save_progress(
                    index + 1
                )

                print("OK")

            except Exception as e:

                print(
                    "ERROR"
                )

                print(
                    f"    {e}"
                )

                # --------------------------------------------
                # On sauvegarde quand même la position
                # de progression.
                #
                # Au prochain lancement, cet objet sera
                # ignoré.
                # --------------------------------------------

                save_progress(
                    index + 1
                )

            time.sleep(
                REQUEST_DELAY
            )

    print()

    print(
        "================================================"
    )

    print(
        "TERMINÉ"
    )

    print(
        "================================================"
    )

    print()

    print(
        f"Fichier : {OUTPUT_FILE}"
    )


# ============================================================
# MAIN
# ============================================================

def main():

    print()
    print(
        "=============================================="
    )
    print(
        " ASTEROID INITIAL STATE BUILDER"
    )
    print(
        "=============================================="
    )

    print()

    print(
        f"EPOCH  : {EPOCH}"
    )

    print(
        f"CENTER : {CENTER}"
    )

    print(
        f"TARGET : {MAX_ASTEROIDS:,}"
    )

    print()

    # --------------------------------------------------------
    # Étape 1
    # --------------------------------------------------------

    build_metadata()

    # --------------------------------------------------------
    # Étape 2
    # --------------------------------------------------------

    process_asteroids()


# ============================================================
# ENTRY POINT
# ============================================================

if __name__ == "__main__":

    main()