# OrbitECS

Simulateur de système solaire interactif en C++/Qt6/QML, basé sur une architecture ECS (Entity Component System). Visualisation temps réel des orbites par gravitation newtonienne, avec zoom et ajout/suppression dynamique de corps célestes (planètes, satellites, astéroïdes).

## Fonctionnalités

- Simulation physique par gravitation universelle (N-corps)
- Visualisation temps réel des trajectoires orbitales
- Zoom et navigation dans la scène
- Ajout et suppression dynamique d'entités (planètes, satellites, astéroïdes) pendant la simulation
- Architecture ECS pure : entities, components et systems découplés

## Stack technique

- **C++** — moteur de simulation (ECS, physique, intégration numérique)
- **Qt6 / QML** — interface graphique et rendu

## Architecture

```
World (C++, logique pure)
 ├─ Components : Position, Velocity, Mass, Radius, Type
 ├─ Systems : GravitySystem, IntegrationSystem
 └─ API exposée à QML : addEntity(), removeEntity(), positions courantes

QML (interface)
 ├─ Canvas de rendu des entités et trajectoires
 ├─ Contrôles zoom / pan
 └─ Actions ajouter / supprimer un corps céleste
```

## Statut

🚧 En développement — projet personnel d'apprentissage de l'architecture ECS.

## Licence

À définir.