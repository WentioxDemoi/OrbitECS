import csv
import io
import requests

EPOCH = "2026-09-08 12:00:00"

# Solar System Barycenter
CENTER = "500@0"

OUT_UNITS = "KM-S"
REF_SYSTEM = "ICRF"
REF_PLANE = "FRAME"

API_URL = "https://ssd.jpl.nasa.gov/api/horizons.api"

# Newtonian gravitational constant
# km^3 kg^-1 s^-2
G_KM3_KG_S2 = 6.67430e-20


# ============================================================
# OBJECTS
# ============================================================

BODIES = {
    "Sun": "10",

    "Mercury": "199",
    "Venus": "299",
    "Earth": "399",
    "Moon": "301",
    "Mars": "499",

    "Jupiter": "599",
    "Saturn": "699",
    "Uranus": "799",
    "Neptune": "899",

    "Pluto": "999",

    # Mars
    "Phobos": "401",
    "Deimos": "402",

    # Jupiter
    "Io": "501",
    "Europa": "502",
    "Ganymede": "503",
    "Callisto": "504",

    # Saturn
    "Mimas": "601",
    "Enceladus": "602",
    "Tethys": "603",
    "Dione": "604",
    "Rhea": "605",
    "Titan": "606",
    "Hyperion": "607",
    "Iapetus": "608",
    "Phoebe": "609",

    # Uranus
    "Ariel": "701",
    "Umbriel": "702",
    "Titania": "703",
    "Oberon": "704",
    "Miranda": "705",

    # Neptune
    "Triton": "801",
    "Nereid": "802",

    # Pluto
    "Charon": "901",
    "Styx": "902",
    "Nix": "903",
    "Kerberos": "904",
    "Hydra": "905",
}


# ============================================================
# PHYSICAL PARAMETERS
#
# GM in km^3/s^2
# radius in km
#
# For N-body calculations, GM is the important quantity.
# ============================================================

PHYSICAL = {

    # --------------------------------------------------------
    # Sun
    # --------------------------------------------------------

    "Sun": {
        "gm_km3_s2": 132712440041.939,
        "radius_km": 695700.0,
    },

    # --------------------------------------------------------
    # Planets
    # --------------------------------------------------------

    "Mercury": {
        "gm_km3_s2": 22031.86855,
        "radius_km": 2439.4,
    },

    "Venus": {
        "gm_km3_s2": 324858.592,
        "radius_km": 6051.8,
    },

    "Earth": {
        "gm_km3_s2": 398600.435436,
        "radius_km": 6371.0084,
    },

    "Mars": {
        "gm_km3_s2": 42828.37362,
        "radius_km": 3389.5,
    },

    "Jupiter": {
        "gm_km3_s2": 126686531.9,
        "radius_km": 69911.0,
    },

    "Saturn": {
        "gm_km3_s2": 37931206.23,
        "radius_km": 58232.0,
    },

    "Uranus": {
        "gm_km3_s2": 5793951.3,
        "radius_km": 25362.0,
    },

    "Neptune": {
        "gm_km3_s2": 6835099.97,
        "radius_km": 24622.0,
    },

    "Pluto": {
        "gm_km3_s2": 869.3,
        "radius_km": 1188.3,
    },

    # --------------------------------------------------------
    # Moon
    # --------------------------------------------------------

    "Moon": {
        "gm_km3_s2": 4902.800,
        "radius_km": 1737.4,
    },

    # --------------------------------------------------------
    # Mars satellites
    # --------------------------------------------------------

    "Phobos": {
        "gm_km3_s2": 0.0007087,
        "radius_km": 11.08,
    },

    "Deimos": {
        "gm_km3_s2": 0.0000962,
        "radius_km": 6.2,
    },

    # --------------------------------------------------------
    # Jupiter satellites
    # --------------------------------------------------------

    "Io": {
        "gm_km3_s2": 5959.916,
        "radius_km": 1821.6,
    },

    "Europa": {
        "gm_km3_s2": 3202.739,
        "radius_km": 1560.8,
    },

    "Ganymede": {
        "gm_km3_s2": 9887.834,
        "radius_km": 2634.1,
    },

    "Callisto": {
        "gm_km3_s2": 7179.289,
        "radius_km": 2410.3,
    },

    # --------------------------------------------------------
    # Saturn satellites
    # --------------------------------------------------------

    "Mimas": {
        "gm_km3_s2": 2.5023,
        "radius_km": 198.2,
    },

    "Enceladus": {
        "gm_km3_s2": 7.210,
        "radius_km": 252.1,
    },

    "Tethys": {
        "gm_km3_s2": 41.21,
        "radius_km": 531.1,
    },

    "Dione": {
        "gm_km3_s2": 73.116,
        "radius_km": 561.4,
    },

    "Rhea": {
        "gm_km3_s2": 153.942,
        "radius_km": 763.8,
    },

    "Titan": {
        "gm_km3_s2": 8978.14,
        "radius_km": 2574.73,
    },

    "Hyperion": {
        "gm_km3_s2": 0.372,
        "radius_km": 135.0,
    },

    "Iapetus": {
        "gm_km3_s2": 120.42,
        "radius_km": 734.5,
    },

    "Phoebe": {
        "gm_km3_s2": 0.552,
        "radius_km": 106.5,
    },

    # --------------------------------------------------------
    # Uranus satellites
    # --------------------------------------------------------

    "Ariel": {
        "gm_km3_s2": 86.4,
        "radius_km": 578.9,
    },

    "Umbriel": {
        "gm_km3_s2": 85.0,
        "radius_km": 584.7,
    },

    "Titania": {
        "gm_km3_s2": 235.5,
        "radius_km": 788.9,
    },

    "Oberon": {
        "gm_km3_s2": 192.4,
        "radius_km": 761.4,
    },

    "Miranda": {
        "gm_km3_s2": 4.4,
        "radius_km": 235.8,
    },

    # --------------------------------------------------------
    # Neptune satellites
    # --------------------------------------------------------

    "Triton": {
        "gm_km3_s2": 1427.6,
        "radius_km": 1353.4,
    },

    "Nereid": {
        "gm_km3_s2": 0.004,
        "radius_km": 170.0,
    },

    # --------------------------------------------------------
    # Pluto satellites
    # --------------------------------------------------------

    "Charon": {
        "gm_km3_s2": 105.88,
        "radius_km": 606.0,
    },

    "Styx": {
        "gm_km3_s2": 0.001,
        "radius_km": 5.0,
    },

    "Nix": {
        "gm_km3_s2": 0.003,
        "radius_km": 23.0,
    },

    "Kerberos": {
        "gm_km3_s2": 0.0001,
        "radius_km": 6.0,
    },

    "Hydra": {
        "gm_km3_s2": 0.001,
        "radius_km": 19.5,
    },
}


# ============================================================
# HORIZONS
# ============================================================

def fetch_body(name, command):

    params = {
        "format": "text",

        "COMMAND": f"'{command}'",

        "OBJ_DATA": "NO",

        "MAKE_EPHEM": "YES",
        "EPHEM_TYPE": "VECTORS",

        "CENTER": f"'{CENTER}'",

        "TLIST": f"'{EPOCH}'",
        "TIME_TYPE": "TDB",

        "REF_SYSTEM": REF_SYSTEM,
        "REF_PLANE": REF_PLANE,

        "OUT_UNITS": OUT_UNITS,

        "VEC_TABLE": "2",
        "VEC_CORR": "NONE",
        "VEC_LABELS": "YES",

        "CSV_FORMAT": "YES",
    }

    response = requests.get(
        API_URL,
        params=params,
        timeout=30,
    )

    response.raise_for_status()

    text = response.text

    if "$$SOE" not in text or "$$EOE" not in text:
        raise RuntimeError(
            f"Horizons n'a pas retourné de vecteur valide "
            f"pour {name}.\n\n{text}"
        )

    data = (
        text
        .split("$$SOE", 1)[1]
        .split("$$EOE", 1)[0]
        .strip()
    )

    line = data.splitlines()[0]

    row = next(
        csv.reader(
            io.StringIO(line)
        )
    )

    # Horizons VEC_TABLE=2:
    #
    # 0 = epoch
    # 1 = Julian date
    # 2 = X
    # 3 = Y
    # 4 = Z
    # 5 = VX
    # 6 = VY
    # 7 = VZ

    physical = PHYSICAL.get(name, {})

    gm = physical.get("gm_km3_s2")
    radius = physical.get("radius_km")

    if gm is not None:
        mass_kg = gm / G_KM3_KG_S2
    else:
        mass_kg = None

    return {
        "name": name,
        "epoch": EPOCH,

        "x_km": float(row[2]),
        "y_km": float(row[3]),
        "z_km": float(row[4]),

        "vx_km_s": float(row[5]),
        "vy_km_s": float(row[6]),
        "vz_km_s": float(row[7]),

        "gm_km3_s2": gm,
        "mass_kg": mass_kg,
        "radius_km": radius,
    }


# ============================================================
# MAIN
# ============================================================

def main():

    print(
        f"Fetching JPL Horizons states at {EPOCH}"
    )

    print(
        f"Center: {CENTER}"
    )

    print()

    results = []

    for name, command in BODIES.items():

        print(
            f"  -> {name} ...",
            end=" "
        )

        try:

            body = fetch_body(
                name,
                command
            )

            results.append(body)

            print("OK")

        except Exception as e:

            print("ERROR")

            print(
                f"     {e}"
            )

    output_file = (
        "solar_system_initial_state.csv"
    )

    fieldnames = [
        "name",
        "epoch",

        "x_km",
        "y_km",
        "z_km",

        "vx_km_s",
        "vy_km_s",
        "vz_km_s",

        "gm_km3_s2",
        "mass_kg",
        "radius_km",
    ]

    with open(
        output_file,
        "w",
        newline="",
        encoding="utf-8",
    ) as f:

        writer = csv.DictWriter(
            f,
            fieldnames=fieldnames,
        )

        writer.writeheader()

        writer.writerows(results)

    print()

    print(
        f"Saved: {output_file}"
    )

    print(
        f"Objects: {len(results)}"
    )


if __name__ == "__main__":
    main()