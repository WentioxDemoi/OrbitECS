# Solar System N-Body Simulation

L'objectif de ce projet est de reproduire, autant que possible, l'expérience proposée par **NASA Eyes on Asteroids**, mais avec une contrainte fondamentale : **les calculs de simulation doivent être effectués localement**.

L'objectif n'est donc pas nécessairement d'intégrer tous les objets connus du Système solaire. Le nombre d'astéroïdes, de comètes et de débris étant extrêmement important, le périmètre sera adapté aux capacités de calcul disponibles, avec l'objectif de conserver une simulation physiquement cohérente et suffisamment précise.

La priorité est de construire une base solide : un Système solaire cohérent, initialisé à partir de données astronomiques fiables, puis propagé localement par un moteur de dynamique gravitationnelle.

---

## 1. Découverte du domaine

### 1.1. Les données astronomiques

Une première piste était d'utiliser le **JPL Small-Body Database (SBDB)** afin de récupérer les éléments orbitaux des objets :

- demi-grand axe `a`
- excentricité `e`
- inclinaison `i`
- longitude du nœud ascendant `Ω`
- argument du périastre `ω`
- anomalie moyenne `M`

Dans ce modèle, les éléments orbitaux permettraient de reconstruire l'état cartésien d'un objet à une epoch donnée.

Il faudrait alors résoudre l'équation de Kepler :

```text
M = E - e sin(E)
```

afin d'obtenir l'anomalie excentrique `E`, puis déterminer :

```text
(a, e, E) → anomalie vraie ν + distance r
```

et finalement convertir les éléments orbitaux en :

```text
position : X Y Z
vitesse  : Vx Vy Vz
```

Cet état initial pourrait ensuite être transmis au moteur N-body.

Cependant, cette approche n'est finalement **pas retenue pour la première version du projet**.

---

## 2. Initialisation avec JPL Horizons

Pour la première version, le projet utilisera directement les données du service **JPL Horizons**.

Horizons permet notamment d'obtenir des vecteurs cartésiens de position et de vitesse pour les objets du Système solaire. JPL utilise lui-même des épémérides numériquement intégrées plutôt que de simples orbites képlériennes pour ses épémérides de haute précision. ([JPL Système Solaire Dynamics](https://ssd.jpl.nasa.gov/horizons/manual.html?utm_source=chatgpt.com))

La première version du projet se concentrera donc sur le Système solaire principal :

- Soleil
- planètes
- lunes naturelles
- éventuellement quelques corps particulièrement massifs ou importants

Les comètes sont volontairement mises de côté pour le moment.

Les astéroïdes seront également ajoutés plus tard, probablement sous forme de **particules tests**, afin d'éviter de faire exploser le coût de calcul.

### Pourquoi ne pas utiliser Kepler ici ?

Parce que nous pouvons directement récupérer auprès de Horizons l'état cartésien d'un objet à une epoch donnée.

Nous n'avons donc pas besoin de faire :

```text
éléments orbitaux
        ↓
équation de Kepler
        ↓
position + vitesse
```

mais directement :

```text
JPL Horizons
        ↓
position + vitesse
        ↓
état initial de la simulation
```

Les éléments képlériens restent néanmoins pertinents pour une éventuelle future intégration de données provenant du SBDB ou d'autres sources.

Ils ne font simplement **pas partie du pipeline actuel**.

---

# 3. Référentiel

Le projet travaillera dans un **référentiel cartésien barycentrique du Système solaire**.

Le barycentre correspond au centre de masse du système considéré. Le Soleil n'est donc pas nécessairement immobile dans ce référentiel : il se déplace lui-même sous l'effet gravitationnel des autres corps massifs. ([JPL Système Solaire Dynamics](https://ssd.jpl.nasa.gov/glossary/barycenter.html?utm_source=chatgpt.com))

C'est un point important : le Soleil ne doit pas être considéré comme un point fixe arbitrairement placé en `(0, 0, 0)` si l'on souhaite conserver une dynamique cohérente.

Les positions et vitesses initiales devront donc utiliser un **même centre de référence et une même epoch**.

Le repère spatial devra rester cohérent avec celui utilisé pour les données astronomiques de départ. JPL utilise notamment l'ICRF pour son système de coordonnées et le TDB comme échelle de temps pour ses épémérides barycentriques. ([JPL Système Solaire Dynamics](https://ssd.jpl.nasa.gov/orbits_doc.html?utm_source=chatgpt.com))

---

# 4. État initial

Chaque objet intégré dans la simulation possédera au minimum :

```text
position
    x
    y
    z

velocity
    vx
    vy
    vz

masse
    m
```

ainsi que les éventuelles informations nécessaires au rendu :

```text
rayon
nom
type
texture
etc.
```

L'epoch de référence sera commune à l'ensemble du système.

L'environnement de simulation sera donc initialisé avec un état de la forme :

```text
SimulationState
│
├── Sun
├── Mercury
├── Venus
├── Earth
├── Moon
├── Mars
├── ...
└── Natural Satellites
```

À partir de ce moment, le moteur physique ne dépend plus de Horizons pour faire évoluer les objets.

---

# 5. Dynamique N-body

Une fois l'environnement initialisé, l'évolution du système sera entièrement calculée localement.

Le principe général est :

```text
État à t
   │
   ▼
Calcul des accélérations gravitationnelles
   │
   ▼
Intégration numérique
   │
   ▼
État à t + Δt
```

Pour deux corps `i` et `j`, l'accélération gravitationnelle est basée sur :

```text
F = G * m_i * m_j / r²
```

et, sous forme vectorielle :

```text
a_i = G * m_j * (r_j - r_i) / |r_j - r_i|³
```

Le moteur devra ensuite intégrer cette accélération afin de mettre à jour les vitesses puis les positions.

---

# 6. Corps massifs et particules tests

Un N-body totalement symétrique nécessite théoriquement de calculer l'influence de chaque corps sur tous les autres.

Pour `N` objets, cela conduit rapidement à une complexité de l'ordre de :

```text
O(N²)
```

Ce modèle devient rapidement trop coûteux si plusieurs milliers ou millions d'astéroïdes sont simulés.

Le projet utilisera donc deux catégories principales.

## Corps massifs

Les corps massifs sont ceux dont l'influence gravitationnelle doit être conservée dans la dynamique générale du système :

- Soleil
- planètes
- lunes naturelles
- éventuellement certains corps particulièrement massifs

Ces corps s'influenceront mutuellement.

```text
Massif ↔ Massif
```

## Petits objets

Les astéroïdes, comètes et éventuellement autres petits objets seront considérés comme des **particules tests**.

Ils subiront l'attraction des corps massifs :

```text
Massif → Petit objet
```

mais leur propre influence gravitationnelle sera ignorée :

```text
Petit objet -X→ Massif
Petit objet -X→ Petit objet
```

Cette approximation est acceptable tant que les objets concernés ont une masse suffisamment faible pour que leur influence sur le reste du système soit négligeable.

Elle permet surtout de conserver un coût de calcul raisonnable lorsque le nombre de petits objets devient important.

Un cas particulier pourra être prévu pour les objets exceptionnellement massifs, par exemple certains gros astéroïdes ou objets transneptuniens, si leur influence devient pertinente pour la simulation.

---

# 7. Intégrateur numérique

Le calcul de l'accélération gravitationnelle et l'intégration temporelle seront séparés.

Le moteur N-body aura donc conceptuellement deux responsabilités :

```text
N-body dynamics
    ↓
calcule les accélérations

Integrator
    ↓
fait évoluer position + vitesse dans le temps
```

Le choix définitif de l'intégrateur reste à déterminer.

Un simple intégrateur d'Euler ne sera probablement pas suffisant pour une simulation orbitale destinée à fonctionner sur de longues périodes, notamment à cause de la dérive numérique de l'énergie et des orbites.

Le projet devra donc évaluer un intégrateur adapté aux systèmes orbitaux, avec une attention particulière portée à :

- la stabilité à long terme ;
- la conservation de l'énergie ;
- la conservation du moment angulaire ;
- le coût CPU ;
- la possibilité d'utiliser des pas de temps suffisamment petits ;
- la stabilité lors des rapprochements avec les planètes ;
- la capacité à intégrer le système en avant comme en arrière dans le temps.

Le choix de l'intégrateur sera donc fait expérimentalement à partir de tests physiques plutôt qu'arbitrairement.

---

# 8. Pas d'intégration et vitesse du temps

Deux notions doivent être strictement distinguées :

### Pas d'intégration (`Δt`)

Le pas d'intégration correspond à la quantité de **temps simulé que l'intégrateur fait progresser à chaque calcul physique**.

Par exemple, avec :

```text
Δt = 1 seconde
```

le moteur calcule l'évolution du système entre `t` et `t + 1 s`.

Avec :

```text
Δt = 60 secondes
```

il calcule l'évolution entre `t` et `t + 60 s`.

Plus `Δt` est important, moins le moteur effectue de calculs pour parcourir une même durée simulée. En contrepartie, l'approximation de l'évolution du système entre deux calculs devient plus importante.

Un pas trop important peut donc provoquer des erreurs numériques : dérive des orbites, mauvaise conservation de l'énergie ou du moment angulaire, voire instabilité dans certaines situations.

Le pas d'intégration ne sera donc **pas choisi en fonction de la vitesse du temps**, mais en fonction de la stabilité et de la précision nécessaires à la simulation.

---

### Vitesse du temps (`timeScale`)

La vitesse du temps représente la quantité de temps simulé qui doit s'écouler pendant une seconde réelle.

Par exemple :

```text
timeScale = 1
```

signifie :

```text
1 seconde réelle = 1 seconde simulée
```

Alors que :

```text
timeScale = 86400
```

signifie :

```text
1 seconde réelle = 1 jour simulé
```

et :

```text
timeScale = 604800
```

signifie :

```text
1 seconde réelle = 7 jours simulés
```

Le `timeScale` ne doit donc **pas être confondu avec `Δt`**.

---

### Relation entre `timeScale` et `Δt`

Si le moteur utilise :

```text
Δt = 1 seconde
```

et que :

```text
timeScale = 604800
```

alors il doit théoriquement effectuer :

```text
604800 / 1 = 604800
```

pas d'intégration par seconde réelle pour avancer de 7 jours simulés.

Avec 38 objets massifs, un N-body naïf représente déjà environ :

```text
38 × 37 / 2 = 703
```

interactions entre paires de corps par pas.

Cela représenterait donc environ :

```text
703 × 604800 ≈ 425 millions
```

de paires à traiter par seconde réelle.

Cette approche serait inutilement coûteuse.

En revanche, si les tests montrent qu'un :

```text
Δt = 60 secondes
```

reste suffisamment précis, la même vitesse de simulation de 7 jours/s nécessite :

```text
604800 / 60 = 10080
```

pas d'intégration par seconde réelle.

Le choix du `Δt` aura donc un impact direct sur les performances.

---

### Le moteur doit calculer aussi vite que possible, mais avec un `Δt` physiquement acceptable

L'objectif n'est pas de limiter arbitrairement le moteur à 60 calculs physiques par seconde.

Les **60 FPS concernent principalement le rendu graphique**.

Le moteur physique doit pouvoir effectuer autant de pas d'intégration que nécessaire et que la machine permet de calculer, tant que chaque pas reste suffisamment petit pour maintenir la précision souhaitée.

On cherche donc à obtenir :

```text
                     précision
                         ▲
                         │
                         │
                  ┌──────┴──────┐
                  │     Δt      │
                  └──────┬──────┘
                         │
                         ▼
                   performances
```

Le `Δt` sera choisi à partir de tests de précision et de stabilité, puis le moteur exécutera les pas nécessaires aussi rapidement que possible.

---

### Timestep adaptatif

À terme, le moteur pourra éventuellement utiliser un pas d'intégration adaptatif.

Lorsque le système évolue lentement :

```text
Δt relativement grand
```

Lorsque deux corps se rapprochent fortement ou que les accélérations changent rapidement :

```text
Δt plus petit
```

Par exemple :

```text
Système calme
    │
    └── Δt = 60 s

Rapprochement important
    │
    └── Δt = 5 s

Situation très dynamique
    │
    └── Δt = 0.5 s
```

Cela permettrait d'éviter de payer en permanence le coût d'un très petit pas alors que le système n'en a pas besoin.

Le timestep adaptatif ne sera cependant adopté qu'après avoir déterminé quel intégrateur est utilisé et quelles contraintes de stabilité celui-ci impose.

---

# 9. Simulation continue et séparation simulation / rendu

La simulation et le rendu graphique seront traités comme deux systèmes distincts.

Le rendu pourra viser environ :

```text
60 FPS
```

mais la simulation physique ne sera pas limitée à :

```text
60 intégrations / seconde
```

Le moteur physique pourra effectuer beaucoup plus de calculs que le nombre d'images affichées.

Une architecture possible sera :

```text
                    Horloge réelle
                          │
                          ▼
                ┌──────────────────┐
                │ Simulation Loop  │
                │                  │
                │ timeScale        │
                │ Δt               │
                │ N-body           │
                │ Integrator       │
                └────────┬─────────┘
                         │
                         ▼
                  Simulation State
                         │
                         ▼
                ┌──────────────────┐
                │   Render Loop    │
                │                  │
                │      ~60 FPS     │
                └──────────────────┘
```

Le rendu graphique ne devra pas bloquer la simulation physique, et inversement.

Le moteur pourra donc, par exemple, effectuer :

```text
10 000 pas physiques / seconde
```

pendant que le renderer affiche :

```text
60 images / seconde
```

Les deux fréquences sont indépendantes.

---

### Budget de calcul

La vitesse du temps demandée par l'utilisateur ne garantit pas que la machine sera capable de la maintenir.

Par exemple, si :

```text
timeScale = 7 jours/s
```

et que le moteur nécessite :

```text
10 080 pas/s
```

mais que le matériel ne peut réellement en calculer que :

```text
7 000 pas/s
```

la simulation ne pourra pas respecter cette vitesse en temps réel.

Le moteur devra donc mesurer sa capacité de calcul.

On peut alors considérer :

```text
temps simulé demandé
        │
        ▼
nombre de pas nécessaires
        │
        ▼
travail physique
        │
        ▼
capacité de calcul disponible
        │
        ▼
temps réel nécessaire
```

L'objectif sera de maintenir la simulation en temps réel lorsque cela est possible, mais **la précision physique restera prioritaire sur la vitesse demandée**.

Si une vitesse temporelle est trop élevée pour le matériel, plusieurs stratégies pourront être envisagées :

- réduire automatiquement la vitesse réelle de simulation ;
- effectuer les calculs plus rapidement en utilisant davantage de ressources ;
- adapter le timestep si la précision le permet ;
- afficher à l'utilisateur que la simulation ne peut pas maintenir le `timeScale` demandé.

Le moteur ne devra pas augmenter arbitrairement `Δt` uniquement pour atteindre une vitesse temporelle donnée si cela dégrade la précision au-delà de la limite acceptable.

---

# 10. Voyage dans le temps

Le temps pourra évoluer dans les deux directions :

```text
t → t + Δt
```

mais également :

```text
t → t - Δt
```

Le moteur devra donc être capable d'intégrer la dynamique :

- vers le futur ;
- vers le passé.

La vitesse du temps pourra être ajustée dynamiquement :

```text
-7 jours/s
-1 jour/s
-1 heure/s
-1 seconde/s
 0
+1 seconde/s
+1 heure/s
+1 jour/s
+7 jours/s
...
```

Le signe du `timeScale` indiquera la direction du temps.

Le moteur devra alors parcourir la durée simulée demandée en utilisant autant de pas d'intégration que nécessaire.

Par exemple :

```text
timeScale = +604800
Δt = 60 s
```

donnera :

```text
604800 / 60 = 10080
```

pas d'intégration pour chaque seconde réelle.

Inversement :

```text
timeScale = -604800
Δt = 60 s
```

fera parcourir les mêmes 10080 pas, mais dans la direction opposée.

---

# 11. Objectif de précision et compromis performance / réalisme

Le projet ne cherche pas à reproduire un logiciel professionnel de mécanique céleste ni à atteindre la précision des intégrateurs utilisés par les organismes spatiaux.

L'objectif est néanmoins de se rapprocher **autant que raisonnablement possible de la réalité physique**, dans les limites :

- de la complexité du problème ;
- des connaissances disponibles ;
- du temps de développement ;
- de la puissance de calcul disponible.

Le matériel cible étant notamment un **MacBook Pro équipé d'une puce Apple M1**, le moteur devra trouver un compromis entre :

```text
                 RÉALISME
                    ▲
                    │
                    │
             précision physique
                    │
                    │
PERFORMANCES ◄──────┼──────► INTERACTIVITÉ
                    │
                    │
                 60 FPS
```

La vitesse de simulation ne sera donc pas une priorité absolue.

Il est préférable d'avoir :

```text
1 jour simulé / seconde réelle
```

avec une trajectoire physiquement cohérente, plutôt que :

```text
7 jours simulés / seconde réelle
```

avec une trajectoire numériquement incorrecte.

Le moteur devra chercher automatiquement ou expérimentalement le meilleur compromis entre :

- taille du timestep ;
- précision ;
- nombre de corps ;
- complexité du calcul N-body ;
- vitesse de simulation ;
- fréquence de rendu ;
- puissance matérielle disponible.

---

# 12. Validation

La simulation ne devra pas être considérée comme correcte simplement parce que les orbites « ont l'air justes ».

Des tests devront être mis en place pour vérifier notamment :

### Influence du timestep

La même simulation devra être exécutée avec différents `Δt` :

```text
1 s
5 s
10 s
30 s
60 s
5 min
10 min
30 min
...
```

Puis les résultats devront être comparés.

L'objectif est de déterminer à partir de quel `Δt` l'erreur devient significative.

---

### Conservation de l'énergie

Sur une simulation isolée, vérifier que l'énergie totale du système ne dérive pas excessivement.

---

### Conservation du moment angulaire

Vérifier que le système conserve correctement son moment angulaire dans les limites imposées par l'intégrateur et les approximations utilisées.

---

### Performance

Mesurer notamment :

```text
nombre de corps
nombre de pas d'intégration / seconde
temps CPU par pas
temps total de simulation
FPS
```

Le benchmark devra permettre de déterminer les limites réelles du matériel.

Par exemple :

```text
38 corps
Δt = 1 s
→ X steps/s

38 corps
Δt = 60 s
→ X steps/s

1000 petits objets
Δt = 60 s
→ X steps/s

10000 petits objets
Δt = 60 s
→ X steps/s
```

Cela permettra de déterminer jusqu'où le moteur peut évoluer avant de devoir introduire des optimisations supplémentaires.

---

### Comparaison avec JPL Horizons

Les positions simulées devront être comparées périodiquement avec celles fournies par JPL Horizons.

Cette comparaison permettra notamment de mesurer l'erreur après :

```text
1 jour
1 mois
1 an
10 ans
...
```

et d'évaluer objectivement l'impact du timestep et de l'intégrateur.

Le but n'est pas nécessairement d'obtenir une correspondance parfaite avec Horizons, mais de comprendre l'erreur introduite par notre modèle numérique et de vérifier qu'elle reste dans les limites fixées par le projet.

# 13. Périmètre initial

La première version ne cherchera pas à reproduire l'intégralité du catalogue du Système solaire.

Le périmètre initial sera plutôt :

```text
Soleil
+
Planètes
+
Lunes naturelles
```

Puis, dans un second temps :

```text
Astéroïdes
```

et éventuellement :

```text
Comètes
Débris
Autres petits objets
```

Les objets seront sélectionnés en fonction de leur intérêt et de leur impact sur les performances.

L'ajout d'un objet ne devra pas nécessiter de modifier le moteur N-body.


Le moteur ne sera considéré comme suffisamment fiable pour accueillir un grand nombre d'astéroïdes qu'après validation de sa stabilité et de sa précision sur le Système solaire de base.