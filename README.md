# myls

`myls` est une réimplémentation académique de la commande GNU/Linux `ls` en C.
Le projet utilise des interfaces C/POSIX classiques : `opendir`, `readdir`,
`lstat`, `stat`, `readlink`, `fnmatch`, `getpwuid`, `getgrgid`, `strftime`,
`ioctl`, `isatty`, etc.

L'objectif est de reproduire les comportements usuels de `ls`, tout en
documentant explicitement les écarts avec GNU coreutils.

## Compilation

```sh
make re
```

## Exécution

```sh
./myls
./myls -la
./myls -lh --sort=time dossier
./myls -R dossier
```

Pour installer la commande dans `~/.local/bin` :

```sh
make install
myls -la
```

Pour désinstaller :

```sh
make uninstall
```

## Tests

```sh
make re
sh tests/test_myls.sh
sh tests/test_options.sh
sh tests/test_valgrind.sh
```

Les tests créent une fixture temporaire avec fichiers normaux, cachés,
exécutables, backup, liens symboliques valides et cassés, lien vers dossier,
FIFO, noms avec espaces/tabulations/retours ligne, fichier commençant par `-`,
dates contrôlées et fichiers `file1`, `file2.o`, `file10`.

Les comparaisons exactes utilisent autant que possible :

```sh
LC_ALL=C /bin/ls --color=never ...
LC_ALL=C ./myls --color=never ...
```

Les zones connues comme non fiables byte-for-byte, comme couleurs, quoting
avancé, alignements et options non supportées, sont testées par présence,
absence, code de retour ou comportement observable.

`tests/test_valgrind.sh` exécute deux scénarios sous valgrind si l'outil est
installé; sinon il affiche `SKIP` et sort avec le code 0.

## Tableau de conformité

États :

- **Complet** : reconnu, modifie le comportement, testé, documenté.
- **Partiel** : reconnu et utile, mais ne reproduit pas GNU `ls` entièrement.
- **Non supporté** : reconnu comme non supporté ou rejeté proprement avec code 2.

| Courte | Longue | État | Test | Commentaire / limite |
| --- | --- | --- | --- | --- |
| `-a` | `--all` | Complet | Oui | Affiche aussi `.` et `..`. |
| `-A` | `--almost-all` | Complet | Oui | Affiche les cachés sauf `.` et `..`. |
| `-d` | `--directory` | Complet | Oui | Affiche le répertoire lui-même. |
| - | `--hide=PATTERN` | Complet | Oui | Utilise `fnmatch` avec `FNM_PERIOD`; ignoré si `-a` ou `-A`. |
| `-I PATTERN` | `--ignore=PATTERN` | Complet | Oui | Utilise `fnmatch` avec `FNM_PERIOD`, comme GNU pour les noms commençant par `.`. |
| `-B` | `--ignore-backups` | Complet | Oui | Ignore les noms finissant par `~`. |
| `-l` | `--format=long`, `--format=verbose` | Complet | Oui | Format long avec `total`, propriétaire, groupe, taille, date, lien `-> cible`. |
| `-1` | `--format=single-column` | Complet | Oui | Une entrée par ligne. |
| `-C` | `--format=vertical` | Partiel | Oui | Colonnes verticales, alignement simplifié. |
| `-x` | `--format=horizontal`, `--format=across` | Partiel | Oui | Colonnes horizontales, alignement simplifié. |
| `-m` | `--format=commas` | Complet | Oui | Entrées séparées par virgule. |
| - | `--zero` | Complet | Oui | Séparateur NUL; implique `-1`, couleur désactivée, caractères de contrôle visibles et quoting literal. |
| `-w COLS` | `--width=COLS` | Complet | Oui | Force la largeur d'affichage. |
| `-T COLS` | `--tabsize=COLS` | Partiel | Oui | Utilisé comme espacement de colonnes, pas comme tabulation GNU complète. |
| `-h` | `--human-readable` | Complet | Oui | Tailles base 1024. |
| - | `--si` | Complet | Non dédié | Tailles base 1000. |
| `-k` | `--kibibytes` | Complet | Non dédié | Blocs de 1024 pour `-s` et `total`. |
| - | `--block-size=SIZE` | Partiel | Non dédié | Supporte entiers et unités simples `K`, `M`, `G`, `KB`, `MB`, `GB`. |
| `-n` | `--numeric-uid-gid` | Complet | Oui | Force UID/GID numériques. |
| `-g` | - | Complet | Non dédié | Format long sans propriétaire. |
| `-o` | - | Complet | Non dédié | Format long sans groupe. |
| `-G` | `--no-group` | Complet | Non dédié | Supprime le groupe en format long. |
| - | `--author` | Partiel | Non dédié | Affiche le propriétaire comme auteur; POSIX ne fournit pas un auteur séparé. |
| `-s` | `--size` | Complet | Oui | Affiche les blocs alloués. |
| `-i` | `--inode` | Complet | Oui | Affiche le numéro d'inode. |
| - | permissions spéciales | Complet | Oui indirect | Gère setuid `s/S`, setgid `s/S`, sticky `t/T`. |
| `-t` | `--sort=time` | Complet | Oui | Tri par temps avec nanosecondes. |
| `-u` | `--time=atime/access/use` | Complet | Oui | Utilise `st_atime`; sans `-l`, trie aussi par atime comme GNU. |
| `-c` | `--time=ctime/status` | Complet | Oui | Utilise `st_ctime`; sans `-l`, trie aussi par ctime comme GNU. |
| - | `--time=mtime/modify/modification` | Complet | Non dédié | Retour au temps de modification. |
| - | `--time=birth/creation` | Non supporté | Oui | Rejeté proprement; birth time non portable POSIX. |
| - | `--full-time` | Complet | Oui | Équivalent pratique à `--time-style=full-iso`. |
| - | `--time-style=full-iso` | Complet | Oui | Format ISO complet simplifié. |
| - | `--time-style=long-iso` | Complet | Oui | Format `YYYY-MM-DD HH:MM`. |
| - | `--time-style=iso` | Complet | Non dédié | Format ISO court. |
| - | `--time-style=locale` | Partiel | Non dédié | Retombe sur le format local par défaut de `strftime`. |
| - | `--time-style=+FORMAT` | Partiel | Oui | Utilise `strftime`; format double avec newline simplifié. |
| `-r` | `--reverse` | Complet | Oui | Inverse le tri actif. |
| `-S` | `--sort=size` | Complet | Oui | Taille décroissante. |
| `-X` | `--sort=extension` | Complet | Oui | Tri par extension. |
| `-U` | `--sort=none` | Complet | Non dédié | Conserve l'ordre du répertoire. |
| `-v` | `--sort=version` | Partiel | Oui | Tri naturel simple, pas l'algorithme GNU complet. |
| - | `--sort=name` | Complet | Non dédié | Tri par nom via `strcoll`. |
| - | `--sort=width` | Complet | Non dédié | Tri par largeur de nom puis nom. |
| - | `--group-directories-first` | Complet | Oui | Groupe aussi les liens vers dossiers. |
| `-f` | - | Partiel | Non dédié | Active `-a` et désactive le tri; ne reproduit pas tous les effets GNU. |
| `-F` | `--classify`, `--classify=WHEN` | Partiel | Oui | Indicateurs GNU principaux; `auto` traité comme `always`. |
| - | `--file-type` | Complet | Oui | Comme `-F` sans `*` sur exécutables. |
| `-p` | `--indicator-style=slash` | Complet | Oui | Ajoute `/` aux dossiers. |
| - | `--indicator-style=none` | Complet | Oui | Désactive les indicateurs. |
| - | `--indicator-style=file-type` | Complet | Oui | Indicateurs sans `*`. |
| - | `--indicator-style=classify` | Complet | Oui | Indicateurs type `-F`. |
| - | `--color`, `--color=always` | Complet | Oui | Couleurs forcées; lit `LS_COLORS` si disponible, sinon fallback intégré. |
| - | `--color=never` | Complet | Oui | Désactive les couleurs. |
| - | `--color=auto` | Complet | Non dédié | Couleur seulement si `stdout` est un terminal. |
| - | `LS_COLORS` | Complet | Oui | Supporte codes de types GNU courants, motifs `*suffix`, `rs`, `lc`, `rc`, `ec`. |
| `-L` | `--dereference` | Complet | Oui | Suit les liens symboliques. |
| `-H` | `--dereference-command-line` | Complet | Non dédié | Suit seulement les liens fournis en argument. |
| - | `--dereference-command-line-symlink-to-dir` | Complet | Non dédié | Suit les liens de ligne de commande vers dossiers. |
| - | lien cassé | Complet | Oui | Affiché en `-l` sous forme `lien -> cible`. |
| `-R` | `--recursive` | Complet | Oui | Parcours récursif, ignore `.` et `..`. |
| `-b` | `--escape` | Complet | Oui | Échappements GNU style `escape`, espaces inclus. |
| `-q` | `--hide-control-chars` | Complet | Oui | Remplace les non imprimables par `?`. |
| - | `--show-control-chars` | Complet | Oui | Annule `-b`/`-q` pour afficher les caractères de contrôle. |
| `-N` | `--literal` | Complet | Oui | Affiche les noms littéralement. |
| `-Q` | `--quote-name` | Complet | Oui | Style GNU `c`, guillemets doubles et échappements C. |
| - | `--quoting-style=WORD` | Complet | Oui | Supporte `literal`, `locale`, `shell`, `shell-always`, `shell-escape`, `shell-escape-always`, `c`, `escape`. |
| `-Z` | `--context` | Non supporté | Oui | SELinux/contexte sécurité non géré; rejet code 2. |
| - | `--lcontext` | Non supporté | Non dédié | SELinux non géré; rejet code 2. |
| - | `--scontext` | Non supporté | Non dédié | SELinux non géré; rejet code 2. |
| - | ACL/SELinux/capability markers | Complet | Oui | En `-l`, affiche `+`, `.`, ou `?` via les xattrs Linux `system.posix_acl_access`, `security.selinux`, `security.capability`. |
| `-D` | `--dired` | Non supporté | Oui indirect | Rejet code 2; indices Emacs dired non générés. |
| - | `--hyperlink`, `--hyperlink=WHEN` | Non supporté | Oui | Rejet code 2; pas de séquences OSC 8. |
| - | `--help` | Complet | Oui | Affiche l'aide et sort avec code 0. |
| - | `--version` | Complet | Oui | Affiche la version et sort avec code 0. |
| - | `--` | Complet | Oui | Fin des options; permet les chemins commençant par `-`. |
| - | options mélangées avec chemins | Complet | Oui | Exemple testé : `myls fichier -l`. |
| - | codes de retour | Complet | Oui | `0` succès, `1` erreurs de traitement, `2` mauvaise utilisation/options non supportées. |

## Synthèse de conformité

- Options / comportements complets : 58
- Options / comportements partiels : 11
- Options / comportements non supportés : 6

Les nombres ci-dessus comptent les lignes fonctionnelles du tableau, pas
uniquement les lettres d'options courtes.

## Différences connues avec GNU `ls`

- Les couleurs lisent `LS_COLORS`; si la variable est absente, `myls` utilise un fallback intégré.
- Le tri `--sort=version` est volontairement simplifié.
- Les colonnes `-C` et `-x` sont lisibles mais pas alignées exactement comme GNU.
- Le quoting GNU courant est reproduit pour les styles documentés; les différences restantes concernent surtout les détails dépendants de locale hors `LC_ALL=C`.
- Les marqueurs ACL/SELinux/capabilities de `-l` sont pris en charge via xattrs; l'affichage détaillé du contexte avec `-Z` reste non supporté.
- Les formats `--time-style=+FORMAT1\nFORMAT2` sont acceptés de façon simplifiée.
- Les détails dépendants de la locale peuvent différer de GNU coreutils.

## Fichiers compilés

Le dépôt ne doit pas versionner :

- `myls`
- `*.o`
- fichiers temporaires

Ces fichiers sont ignorés par `.gitignore`.

## Auteurs

Equipe 6.
