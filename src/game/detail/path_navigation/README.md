# path_navigation

AI pathfinding *session* management - following a pre-computed path on the navmesh.

Depends on `game/detail/pathfinding/` for the underlying A* algorithms.

## Files

- **`path_helpers.hpp`** - shared constants and small inline helpers used across the session layer:
  `CELL_REACH_EPSILON`, `DEVIATION_CHECK_RANGE_V`, `is_within_cell`, `get_current_path_target`, `is_on_portal_cell`

- **`start_pathfinding_to.hpp`** - kicks off a new `ai_pathfinding_state` toward a target transform;
  calls `find_path_across_islands_many` and refuses to start while the bot is on a portal cell

- **`advance_path_if_cell_reached.hpp`** - advances the active path's `node_index` when the bot
  reaches a cell centre (supports cardinal and diagonal movement, rerouting overlay, portal hold)

- **`check_path_deviation.hpp`** - detects when the bot has strayed off the path and either
  snaps `node_index` to the nearest on-path cell or calculates a rerouting sub-path

- **`get_pathfinding_movement_direction.hpp`** - converts the current path node into a movement
  direction vector and a smoothed crosshair offset (with stuck-rotation fallback)

- **`debug_draw_pathfinding.hpp`** - draws the main path (green) and rerouting overlay (yellow)
  into `DEBUG_LOGIC_STEP_RECTS/LINES` when `DEBUG_DRAWING.draw_ai_info` is set

- **`navigate_pathfinding.hpp`** - **main entry point**: calls all of the above in order,
  returns a `navigate_pathfinding_result` with `movement_direction`, `crosshair_offset`,
  `can_sprint`, `nearing_end`, and `path_completed`
