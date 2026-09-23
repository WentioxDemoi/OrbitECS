# OrbitECS

Simulateur de système solaire interactif en C++/Qt6/QML, basé sur une architecture inspirée de l'**Entity Component System (ECS)**. Visualisation temps réel des orbites par gravitation newtonienne.

## Fonctionnalités

- Simulation physique par gravitation universelle (N-corps)
- Visualisation temps réel des trajectoires orbitales
- Navigation 3D dans la scène
- Zoom de la caméra
- Architecture séparant les données, la physique et le rendu
- Simulation exécutée dans un thread dédié
- Rendu et interface graphique avec Qt6/QML

## Contrôles

| Action | Contrôle |
|---|---|
| Orienter la caméra | **Clic gauche + glisser** |
| Déplacement horizontal | **WASD** ou **Flèches directionnelles** |
| Monter | **R** ou **PageUp** |
| Descendre | **F** ou **PageDown** |
| Accélérer les déplacements | **Shift** |
| Zoom | **Molette** |

## Stack technique

- **C++** — moteur de simulation (ECS, physique, intégration numérique)
- **Qt6 / QML** — interface graphique et rendu 3D
- **Qt Quick 3D** — visualisation des corps célestes et instancing GPU
- **Multithreading** — séparation du calcul physique et du rendu

## Architecture

```text
World (C++, logique de simulation)
 ├─ Components : Position, Velocity, Mass, Radius, Type
 ├─ Systems : GravitySystem, IntegrationSystem
 └─ Données de simulation

             ↓

Backend / Simulation Thread
 └─ Calcul physique et évolution de l'état du système

             ↓

BufferExchange
 └─ Échange des états entre le thread de simulation et le frontend

             ↓

Frontend
 ├─ Interpolation des positions
 ├─ Préparation des données de rendu
 └─ Exposition des données à QML

             ↓

QML / Qt Quick 3D
 ├─ Rendu 3D des entités
 ├─ Instancing GPU
 ├─ Contrôleur de caméra
 └─ Contrôles utilisateur
```

## À propos de l'architecture ECS

Le projet est actuellement une **première version inspirée d'une architecture ECS**, mais il ne s'agit pas encore d'un ECS totalement conforme à cette approche.

Actuellement, les composants sont stockés dans des tableaux et les différents composants d'une même entité sont associés grâce à leur **index dans ces tableaux** :

```text
Position[0] ←→ Velocity[0] ←→ Mass[0]
Position[1] ←→ Velocity[1] ←→ Mass[1]
Position[2] ←→ Velocity[2] ←→ Mass[2]
```

C'est une architecture ECS en SoA qui est utilisée (Struct of Arrays).
Nous n'utilisons donc pas encore de véritable **`EntityID`** permettant d'identifier indépendamment chaque entité et de référencer ses composants.

Cette approche est volontaire pour cette première version afin de garder la structure de données simple et performante, mais elle constitue une limite de l'architecture actuelle.

De même, **l'ajout et la suppression dynamique de corps célestes pendant la simulation ne sont pas encore implémentés**. Les entités sont actuellement définies lors de l'initialisation de la simulation et leur nombre reste fixe pendant son exécution.

Une évolution future pourra introduire :

- un système de véritables `EntityID` ;
- la gestion du cycle de vie des entités ;
- l'ajout et la suppression dynamique de composants ;
- l'ajout et la suppression de corps pendant la simulation ;
- une meilleure gestion de la correspondance entre entités et composants.

## Performances sur macOS

Sur macOS, les performances du thread de simulation diminuaient après environ **30–60 secondes lorsque la fenêtre n'était plus au premier plan**.

Le problème apparaissait avec `QGuiApplication` et sa boucle événementielle, mais pas avec `QCoreApplication`.

Le comportement semblait lié à la **gestion énergétique et au throttling des applications fenêtrées en arrière-plan sur macOS**, notamment à **App Nap / Power Management**.

Ce comportement a été identifié comme une limitation liée à l'environnement macOS plutôt qu'à un problème majeur dans le moteur de simulation lui-même.

## Statut

🚧 **En développement** — projet personnel d'apprentissage de l'architecture ECS, de la simulation physique N-corps et du rendu 3D avec Qt6.

L'architecture actuelle constitue une première implémentation fonctionnelle, mais plusieurs éléments restent à faire évoluer, notamment la gestion des `EntityID` et le cycle de vie dynamique des entités.

## Licence

À définir.