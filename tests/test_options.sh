#!/bin/bash

# ============================================================
# Script de test automatique pour options.c
# Usage : bash tests/test_options.sh
# ============================================================

PASS=0
FAIL=0
BINARY="./myls"

# Couleurs
GREEN=$(tput setaf 2 2>/dev/null || echo "")
RED=$(tput setaf 1 2>/dev/null || echo "")
NC=$(tput sgr0 2>/dev/null || echo "")

# Fonction de test
check() {
    local desc="$1"
    local cmd="$2"
    local expected_output="$3"
    local expected_code="$4"

    actual_output=$(eval "$cmd" 2>&1)
    actual_code=$?

    if [[ "$actual_output" == *"$expected_output"* ]] && \
       [[ "$actual_code" -eq "$expected_code" ]]; then
        echo -e "${GREEN}PASS${NC} : $desc"
        ((PASS++))
    else
        echo -e "${RED}FAIL${NC} : $desc"
        echo "       Commande  : $cmd"
        echo "       Attendu   : '$expected_output' (code $expected_code)"
        echo "       Obtenu    : '$actual_output' (code $actual_code)"
        ((FAIL++))
    fi
}

# Vérification que le binaire existe
if [ ! -f "$BINARY" ]; then
    echo "ERREUR : binaire '$BINARY' introuvable. Lance 'make' d'abord."
    exit 1
fi

echo "============================================================"
echo " Tests automatiques - module options.c"
echo "============================================================"
echo ""

# ── Groupe 1 : Comportement de base ─────────────────────────
echo "--- Groupe 1 : Comportement de base ---"
check "Aucun argument"        "./myls"       "Répertoire courant" 0
check "Option -l seule"       "./myls -l"    "Répertoire courant" 0
check "Option -a seule"       "./myls -a"    "Répertoire courant" 0
check "Options combinées -la" "./myls -la"   "Répertoire courant" 0
check "Options combinées -lah""./myls -lah"  "Répertoire courant" 0
check "Options combinées -laR""./myls -laR"  "Répertoire courant" 0
echo ""

# ── Groupe 2 : Chemins ──────────────────────────────────────
echo "--- Groupe 2 : Chemins ---"
check "Chemin seul"           "./myls /tmp"        "Chemin : /tmp" 0
check "Option + chemin"       "./myls -l /tmp"     "Chemin : /tmp" 0
check "Deux chemins (1er)"    "./myls /tmp /etc"   "Chemin : /tmp" 0
check "Deux chemins (2e)"     "./myls /tmp /etc"   "Chemin : /etc" 0
echo ""

# ── Groupe 3 : Options longues ───────────────────────────────
echo "--- Groupe 3 : Options longues ---"
check "--all"             "./myls --all"            "Répertoire courant" 0
check "--almost-all"      "./myls --almost-all"     "Répertoire courant" 0
check "--recursive"       "./myls --recursive"      "Répertoire courant" 0
check "--human-readable"  "./myls --human-readable" "Répertoire courant" 0
check "--reverse"         "./myls --reverse"        "Répertoire courant" 0
check "--inode"           "./myls --inode"          "Répertoire courant" 0
check "--numeric-uid-gid" "./myls --numeric-uid-gid""Répertoire courant" 0
check "--no-group"        "./myls --no-group"       "Répertoire courant" 0
check "--dereference"     "./myls --dereference"    "Répertoire courant" 0
check "--directory"       "./myls --directory"      "Répertoire courant" 0
echo ""

# ── Groupe 4 : --color ───────────────────────────────────────
echo "--- Groupe 4 : --color ---"
check "--color=always"    "./myls --color=always"   "Répertoire courant" 0
check "--color=never"     "./myls --color=never"    "Répertoire courant" 0
check "--color=auto"      "./myls --color=auto"     "Répertoire courant" 0
check "--color sans valeur""./myls --color"         "Répertoire courant" 0
echo ""

# ── Groupe 5 : Cas limites ───────────────────────────────────
echo "--- Groupe 5 : Cas limites ---"
check "-- fin des options"    "./myls --"        "Répertoire courant" 0
check "-- suivi d'un chemin"  "./myls -- /tmp"   "Chemin : /tmp"      0
check "-l puis chemin"        "./myls -l /tmp"   "Chemin : /tmp"      0
echo ""

# ── Groupe 6 : --help et --version ───────────────────────────
echo "--- Groupe 6 : --help et --version ---"
check "--help"    "./myls --help"    "Utilisation" 0
check "--version" "./myls --version" "myls version" 0
echo ""

# ── Groupe 7 : Gestion des erreurs ───────────────────────────
echo "--- Groupe 7 : Gestion des erreurs ---"
check "Option invalide -z"      "./myls -z"    "option invalide" 2
check "Option invalide -@"      "./myls -@"    "option invalide" 2
check "Option longue inconnue"  "./myls --xyz" "inconnue"        2
echo ""

# ── Groupe 8 : Options avec valeur ───────────────────────────
echo "--- Groupe 8 : Options avec valeur ---"
check "--sort=size"         "./myls --sort=size"          "Répertoire courant" 0
check "--sort=time"         "./myls --sort=time"          "Répertoire courant" 0
check "--sort=none"         "./myls --sort=none"          "Répertoire courant" 0
check "--sort=extension"    "./myls --sort=extension"     "Répertoire courant" 0
check "--sort=version"      "./myls --sort=version"       "Répertoire courant" 0
check "--time=atime"        "./myls --time=atime"         "Répertoire courant" 0
check "--time=ctime"        "./myls --time=ctime"         "Répertoire courant" 0
check "--block-size=1024"   "./myls --block-size=1024"    "Répertoire courant" 0
check "--width=80"          "./myls --width=80"           "Répertoire courant" 0
check "--hide=*.txt"        "./myls --hide=*.txt"         "Répertoire courant" 0
check "--ignore=*.log"      "./myls --ignore=*.log"       "Répertoire courant" 0
check "--time-style=full-iso" "./myls --time-style=full-iso" "Répertoire courant" 0
check "--tabsize=4"         "./myls --tabsize=4"          "Répertoire courant" 0
echo ""

# ── Résumé ───────────────────────────────────────────────────
echo "============================================================"
printf " Résultats : %s%d PASS%s / %s%d FAIL%s\n" \
    "$GREEN" "$PASS" "$NC" "$RED" "$FAIL" "$NC"
echo "============================================================"

[ "$FAIL" -eq 0 ] && exit 0 || exit 1
