# 08. Phase 4 - Creator Workshop

## Objective

Turn PeterCraft from a great game into a great bridge toward programming and game development.

## Outputs

- workshop creator progression
- tinker mode
- logic mode
- code mode
- safe reset and sandboxing
- creator tutorials and challenges

## Core principle

The workshop is a ladder:

1. **Tinker** — change values
2. **Logic** — change rules and flows
3. **Code** — write tiny scripts
4. **Build** — make simple missions or encounters

The player should move upward only when they feel curious, not pressured.

## Step-by-step plan

### Step 1: Build Tinker Mode
Expose safe variables such as:
- companion follow distance
- loot priority
- mission timer length in tutorial scenarios
- extraction countdown
- enemy alert delay in learning spaces

Requirements:
- sliders, toggles, small presets
- preview + apply + undo
- reset to default

Acceptance:
- edits are safe
- changes are visible quickly
- no setting can break a save or soft-lock a mission

### Step 2: Build Logic Mode
Expose simple rule chains such as:
- if player health is low, companion retreats or helps
- if rare loot is found, companion marks it
- if extraction is active, companion prioritizes cover

Requirements:
- node or card-based editing
- limited safe vocabulary
- visual examples and templates
- validation before activation

Acceptance:
- players can complete a guided “change the rule” mission
- invalid logic is caught with friendly feedback

### Step 3: Build Code Mode
Expose tiny script hooks, not a full engine API.

Allowed examples:
- scoring a custom mission
- choosing between two companion priorities
- customizing a tutorial event message
- reacting to loot rarity

Requirements:
- tiny curated API
- syntax guidance
- run/validate button
- sandbox execution
- timeout and safety guards
- clear error messages

Acceptance:
- a child can complete the first script with in-game guidance
- bad scripts cannot crash the game

### Step 4: Build creator save isolation
Store creator edits in isolated save/content space.

Requirements:
- versioned user content
- rollback on invalid content
- duplicate / clone / reset
- never overwrite shipped defaults

Acceptance:
- all user-made content can be safely disabled or restored

### Step 5: Build creator tutorials
Create scripted lessons:
- “change a value”
- “change a rule”
- “read an explanation”
- “write your first tiny script”
- “build a mini mission from a template”

Acceptance:
- each lesson is short and rewarding
- each lesson ends with a playable result

### Step 6: Build mini mission templates
Allow the child to assemble small missions from safe templates:
- choose room
- choose loot goal
- choose one enemy group
- choose extraction point
- choose reward

Acceptance:
- missions are valid by construction or validated before save
- a player can launch a custom mini mission in under 5 minutes

### Step 7: Build comparison and replay tools
Add:
- before/after behavior comparison
- replay snippets of companion choices
- “your change caused this” summaries

Acceptance:
- the workshop reinforces learning through evidence, not guesswork

### Step 8: Build adult/mentor support surfaces
Add:
- hidden advanced notes or mentor view
- exportable creator summaries
- safe recovery tools for broken custom content
- progress overview for learning milestones

Acceptance:
- adults can support without needing full engine knowledge

## Design rules for creator tools

- never punish experimentation
- always support undo
- validate before apply
- keep vocabulary child-friendly
- show examples before blank canvases
- expose a small set of high-value concepts first
- gradually increase expressive power

## Exit criteria

Phase 4 is complete when:
- the workshop is genuinely enjoyable
- creator tools feel safe and empowering
- at least one playtester in the target age range successfully completes all three layers: value edit, rule edit, tiny script
