# path_navigation

AI pathfinding *session* management — following a pre-computed path on the navmesh.

Depends on `game/detail/pathfinding/` for the underlying A* algorithms.

## Files

- **`path_helpers.hpp`** — shared constants and small inline helpers used across the session layer:
  `CELL_REACH_EPSILON`, `DEVIATION_CHECK_RANGE_V`, `is_within_cell`, `get_current_path_target`, `is_on_portal_cell`

- **`start_pathfinding_to.hpp`** — kicks off a new `ai_pathfinding_state` toward a target transform;
  calls `find_path_across_islands_many` and refuses to start while the bot is on a portal cell

- **`advance_path_if_cell_reached.hpp`** — advances the active path's `node_index` when the bot
  reaches a cell centre (supports cardinal and diagonal movement, rerouting overlay, portal hold)

- **`check_path_deviation.hpp`** — detects when the bot has strayed off the path and either
  snaps `node_index` to the nearest on-path cell or calculates a rerouting sub-path

- **`get_pathfinding_movement_direction.hpp`** — converts the current path node into a movement
  direction vector and a smoothed crosshair offset (with stuck-rotation fallback)

- **`debug_draw_pathfinding.hpp`** — draws the main path (green) and rerouting overlay (yellow)
  into `DEBUG_LOGIC_STEP_RECTS/LINES` when `DEBUG_DRAWING.draw_ai_info` is set

- **`navigate_pathfinding.hpp`** — **main entry point**: calls all of the above in order,
  returns a `navigate_pathfinding_result` with `movement_direction`, `crosshair_offset`,
  `can_sprint`, `nearing_end`, and `path_completed`

---

## Pathfinding architecture

### `pathfinding/` — finds the path

- `cell_to_world` / `world_to_cell` — coordinate conversion
- `find_island_for_position` — which navmesh island contains a world point
- `find_islands_connection` — BFS over island graph → next-hop island
- `find_best_portal_from_to` — best portal by distance heuristic
- `find_path_within_island` — 8-directional A* within one island
- `find_path_across_islands_many` — multi-island path (returns path to next portal only)
- `find_path_across_islands_many_full` — repeats the above to produce the full route (visualisation)
- `find_closest_walkable_cell` / `find_closest_unoccupied_cell` — BFS fallbacks
- `find_random_unoccupied_*` — random spawn helpers
- `find_bomb_pathfinding_target` — resolves bomb position to the nearest walkable tile

### `path_navigation/` — follows the path (per-tick AI session logic)

- `start_pathfinding_to` → creates a fresh `ai_pathfinding_state`
- `advance_path_if_cell_reached` → ticks node index forward
- `check_path_deviation` → detects straying and triggers rerouting
- `get_pathfinding_movement_direction` → computes movement + crosshair vectors
- `debug_draw_pathfinding` → visualises active path
- `navigate_pathfinding` **(main entry)** → calls all of the above, returns `navigate_pathfinding_result`
