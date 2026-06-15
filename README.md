# myls

`myls` est une réimplémentation académique partielle de `ls` GNU/Linux en C.
Le code utilise les interfaces C/POSIX classiques pour lire les répertoires,
statuer les fichiers, formater les métadonnées et afficher les entrées.

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

Pour lancer `myls` depuis n'importe quel répertoire :

```sh
make install
myls -la
```

Par défaut, `make install` installe le binaire dans `~/.local/bin`, qui doit
être présent dans la variable `PATH`.

## Options supportées

Sélection et chemins :

- `-a`, `--all`
- `-A`, `--almost-all`
- `-d`, `--directory`
- `--hide=PATTERN`
- `--ignore=PATTERN`
- `-B`, `--ignore-backups`

Formats :

- `-l`
- `-1`
- `-C`
- `-x`
- `-m`
- `--zero`
- `-w N`, `--width=N`
- `-T N`, `--tabsize=N`

Métadonnées :

- `-h`, `--human-readable`
- `--si`
- `-k`, `--kibibytes`
- `--block-size=N`
- `-n`, `--numeric-uid-gid`
- `-g`
- `-o`
- `-G`, `--no-group`
- `-s`, `--size`
- `-i`, `--inode`

Dates :

- `-t`
- `-u`
- `-c`
- `--time=atime`, `--time=access`, `--time=use`
- `--time=ctime`, `--time=status`
- `--full-time`
- `--time-style=long-iso`
- `--time-style=full-iso`
- `--time-style=iso`

Tri :

- tri par nom par défaut avec `strcoll()`
- `-S`, `--sort=size`
- `-t`, `--sort=time`
- `-X`, `--sort=extension`
- `-U`, `--sort=none`
- `-v`, `--sort=version`
- `--sort=name`
- `--sort=width`
- `-r`
- `--group-directories-first`
- `-f`

Types, couleurs et liens :

- `-F`, `--classify`
- `--file-type`
- `-p`, `--indicator-style=slash`
- `--color=never`
- `--color=always`
- `--color=auto`
- `-L`, `--dereference`
- `-H`, `--dereference-command-line`
- `--dereference-command-line-symlink-to-dir`
- affichage `lien -> cible` avec `-l`

Récursivité :

- `-R`, `--recursive`

## Options simplifiées ou partielles

- `--sort=version` utilise un tri naturel simple, pas l’algorithme complet GNU.
- Les colonnes `-C` et `-x` respectent la largeur demandée mais ne reproduisent
  pas tous les détails d’alignement de GNU `ls`.
- Les couleurs sont fixes et simplifiées. `LS_COLORS` n’est pas interprété.
- `--author` affiche l’auteur comme le propriétaire, car POSIX ne fournit pas
  une métadonnée auteur séparée.
- `-b`, `-q`, `-N`, `-Q` ont une gestion simple des noms affichés.

## Options reconnues mais non supportées complètement

- `-D`, `--dired`
- `-Z`
- `--hyperlink`

Ces options sont parsées pour compatibilité de ligne de commande, mais le
comportement avancé GNU correspondant n’est pas reproduit.

## Tests

```sh
make re
sh tests/test_myls.sh
```

Les tests créent un dossier temporaire, génèrent fichiers normaux, cachés,
exécutables, liens symboliques, sous-répertoires et fichiers backup, puis
comparent `myls` à `ls` sur les comportements principaux.

## Limites connues

`myls` vise une reproduction raisonnable de `ls` pour un projet académique. Les
détails dépendants de la locale, des couleurs GNU, des ACL/capacités, des
contextes SELinux et de certains alignements exacts ne sont pas reproduits
byte-for-byte.

## Auteurs

Equipe 6.
