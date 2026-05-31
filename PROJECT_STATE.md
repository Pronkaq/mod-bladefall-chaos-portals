# BladeFall Chaos Portals Project State

## Module

Standalone AzerothCore module:

```text
mod-bladefall-chaos-portals
MVP design
One global encounter at a time.

Wait 10 minutes after completion before attempting another encounter.

Select one random eligible open-world player.

The same player may be selected again.

Spawn one Chaos Portal near the selected player.

Spawn 1-5 melee Chaos Invaders.

Invaders match the selected player's level.

Invaders deal 90% of their normal physical melee damage.

The selected player has 60 seconds to defeat all invaders.

Other players may assist.

Only the selected player receives the reward.

Reward: 1 Chaos Shard for each spawned invader.

Failure: despawn remaining creatures and portal.

Failure debuff: -10% to all stats for 60 seconds.

Selected templates
Portal
Custom ID: 900001

Name: BladeFall Chaos Portal

Clone source: 177193 (Doom Portal)

Display ID: 4972

Creature
Custom ID: 900001

Name: Chaos Invader

Clone source: 17261 (Restless Skeleton)

Reuse existing skeleton models.

Reward item
Custom ID: 900001

Name: Chaos Shard

Russian name: Осколок хаоса

Reward amount: one shard per spawned invader.

Template source: not selected yet.

Failure debuff
A client-known spell is not selected yet.

The first narrow search did not find a suitable spell.
Spell 24899 (Heart of the Wild Bear Effect) is not suitable because its
EffectMiscValue_1 is 2, so it affects a specific stat rather than all stats.

Run the expanded query that includes aura types 29, 80 and 137 with
EffectMiscValue = -1.

IDs reserved for prototype
The same numeric ID can be reused because these are different DB tables:

Chaos Shard item:         900001
Chaos Portal gameobject:  900001
Chaos Invader creature:   900001
