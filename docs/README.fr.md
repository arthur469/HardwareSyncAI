# Hardware Sync AI — Français

Projet dérivé indépendant de [Hardware Sync](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin), sous GPL-2.0-or-later. L'interface Qt, les palettes, mesures et sélections de zones sont conservées ; la couche IA ajoute des effets conditionnés par la présence de modèles Ollama. Ce n'est pas une version officielle d'OpenRGB.

## Installation

Télécharger le fichier `.so` Linux x86_64 / Qt 6 / API de plugin 5 depuis Releases. Dans OpenRGB : **Settings → Plugins → Install plugin**, sélectionner le `.so`, puis activer Hardware Sync AI. Ne pas faire piloter les mêmes appareils par le plugin original et cette version simultanément.

Vérifier les versions Qt/libc dans BUILD-INFO : la première compilation cible Linux récent et ne convient pas à un hôte Qt 5. Aucune compilation Windows/macOS n'est annoncée validée. Python 3 et les bibliothèques Linux indiquées dans le README sont nécessaires ; aucun module Python externe, compte ou fichier personnel n'est requis. Les mesures NVIDIA utilisent `nvidia-smi` ; Ollama est facultatif et interrogé uniquement en local. Les autres entrées de capteurs sont découvertes par libsensors comme dans Hardware Sync.

La première ouverture est en pause et sans appareil associé. Ajouter une mesure, choisir un capteur, cocher les appareils/zones, régler la palette, lancer la mesure et reprendre explicitement les écritures. La configuration propre à chaque utilisateur reste dans son dossier OpenRGB ; aucune identité matérielle n'est livrée.

## Conditions et effets

La couleur normale suit la mesure et sa palette. Une seule couleur dans la palette donne une couleur fixe. **Breathing** ajoute une respiration lumineuse ; **Pulse** une impulsion suivie d'une décroissance ; **No effect** n'ajoute aucun overlay.

**Always** applique l'overlay en permanence. **Model GPU**, **Model CPU** et **Model RAM** suivent les allocations déclarées par Ollama ; leurs précisions sont dans les infobulles. Un modèle chargé peut être inactif : ce n'est pas une mesure du calcul IA en cours. Les allocations CPU/RAM restent des estimations. Une condition inconnue fige les écritures de l'effet concerné.

Quand la condition est fausse, seul l'overlay est retiré : la couleur normale liée à la mesure reste active. Choisir « No effect » retire les animations ; la condition est alors sans effet.

Les modes **Device: …** proviennent réellement des appareils. Seuls les modes communs à tous les appareils sélectionnés sont proposés, et uniquement pour une sélection de toutes leurs zones non vides. Ils conservent leurs paramètres OpenRGB et leur cadence matérielle ; la période de l'overlay logiciel ne règle pas le firmware. Une condition fausse rétablit le mode précédent.

Le breathing logiciel utilise au moins 30 images/s et un lissage temporel. Le seuil de variation de palette, réglable, évite les changements dus aux petites fluctuations (1 % par défaut). Les modes noir/OFF, changements manuels et conflits entre règles restent protégés. Un rescan met les écritures en pause. Une animation native autonome peut continuer pendant cette pause ; l'arrêter via son mode dans OpenRGB.

## Conservation des associations

Un appareil absent ne perd pas ses associations. Au retour, une identité suffisamment fiable et unique restaure les zones et réglages. Les identités ambiguës, nombres de LED modifiés et zones non reconnues restent mémorisés sans attribution arbitraire. La désélection volontaire supprime l'association ; « Oublier les cibles indisponibles » supprime celles en attente dans la mesure courante. Les appareils physiquement identiques sans identifiant fiable ne peuvent pas toujours être distingués automatiquement.

## Sources, historique et contributions

Voir le README anglais pour la compilation. L'historique et la licence du projet initial sont conservés ; la branche `upstream/master` marque l'import et `feature/hardware-sync-ai` porte nos ajouts. Ne pas fusionner automatiquement les mises à jour.

Développement assisté par IA déclaré. Aucune soumission n'est faite aux développeurs OpenRGB. Vérifier les politiques IA avant toute contribution à un autre projet. Ne jamais publier de profils personnels, identifiants d'appareil, comptes ou diagnostics privés.
