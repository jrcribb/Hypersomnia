available_resources = {
	outfits = {
		"white",
		"basic"
	},
	
	weapon_types = {
		"barehands",
		"rifle",
		"melee"
	}
}

sprite_tokens = {
	BODYPART = 1,
	OUTFIT = 2,
	ACTIVITY = 3,
	WEAPON = 4
}

function create_all_legs_sets(sprite_library)
	local legs_sets = {}
	
	for k, outfit in ipairs(available_resources.outfits) do
		legs_sets[outfit] = create_legs_set_for(sprite_library, outfit)
	end
	
	return legs_sets
end

function create_all_torso_sets(sprite_library)
	local torso_sets = {}
	
	for k, outfit in ipairs(available_resources.outfits) do
		torso_sets[outfit] = {}
		for j, weapon_type in ipairs(available_resources.weapon_types) do
			torso_sets[outfit][weapon_type] = create_torso_set_for(sprite_library, outfit, weapon_type)
		end
	end
	
	return torso_sets
end

-- returns newly created animation set and an animation container
function create_legs_set_for(sprite_library, outfit)
	local animation_container = {}
	local frames = sprite_library["legs"][outfit]

	local new_legs_animation = create_animation {
		frames = {
			{ model = nil, duration_ms = 2 },
			{ model = { image = frames["1"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = frames["2"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = frames["3"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = frames["4"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = frames["5"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = frames["4"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = frames["3"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = frames["2"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = frames["1"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = false end },
			{ model = nil, duration_ms = 2 },                                                                    
			{ model = { image = frames["1"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = true end },
			{ model = { image = frames["2"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = true end },
			{ model = { image = frames["3"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = true end },
			{ model = { image = frames["4"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = true end },
			{ model = { image = frames["5"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = true end },
			{ model = { image = frames["4"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = true end },
			{ model = { image = frames["3"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = true end },
			{ model = { image = frames["2"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = true end },
			{ model = { image = frames["1"] }, duration_ms = 20, callback = function(subject) subject.render.flip_vertically = true end }
		},
		loop_mode = animation.REPEAT
	}
		
	table.insert(animation_container, new_legs_animation)
	
	local new_animation_set = create_animation_set {
		animations = { 
			{ event = animation_events.MOVE, animation_response = new_legs_animation },
			{ event = animation_events.MOVE_CW, animation_response = new_legs_animation },
			{ event = animation_events.MOVE_CCW, animation_response = new_legs_animation }
		}
	}
	
	return { set = new_animation_set, animations = animation_container }
end

-- returns newly created animation set and an animation container
-- things get a little complicated here
function create_torso_set_for(sprite_library, outfit, weapon_type)
	local walk_frames  = sprite_library["torso"][outfit]["walk"][weapon_type]
	local shoot_frames = sprite_library["torso"][outfit]["shoot"][weapon_type]
	
	local animation_container = {}
	local animation_set;
		
	local function set_offsets(subject, group, index)
		local entity = subject.script
		local wielded = entity.wield.wielded_items[components.wield.keys.PRIMARY_WEAPON]
		
		if not wielded then return end
		local offset_info = entity.wield.wield_offsets[wielded.item.outfit_type]
		--local head_info = npc_info.wield_offsets["HEAD"]
		--
		--npc_info.head_entity.chase.rotation_orbit_offset = head_info[group][index].pos
		
		if not offset_info  or not offset_info[group] then return end
		
		wielded.cpp_entity.chase.rotation_offset = offset_info[group][index].rotation
		wielded.cpp_entity.chase.rotation_orbit_offset = offset_info[group][index].pos
		
		if offset_info[group][index].flip ~= nil then
			wielded.cpp_entity.render.flip_vertically = offset_info[group][index].flip
		else 
			wielded.cpp_entity.render.flip_vertically = false
		end
	end
		
	local function frame(n, should_flip, group)
		return function(subject)
			set_offsets(subject, group, n)
			subject.render.flip_vertically = should_flip
		end
	end
	
	if weapon_type == "barehands" then
		local new_walk_animation = create_animation {
			frames = {
				{ model = { image = walk_frames["1"] }, duration_ms = 20, callback = frame(1, false, "walk") },
				{ model = { image = walk_frames["2"] }, duration_ms = 20, callback = frame(2, false, "walk") },
				{ model = { image = walk_frames["3"] }, duration_ms = 20, callback = frame(3, false, "walk") },
				{ model = { image = walk_frames["4"] }, duration_ms = 20, callback = frame(4, false, "walk") },
				{ model = { image = walk_frames["5"] }, duration_ms = 20, callback = frame(5, false, "walk") },
				{ model = { image = walk_frames["4"] }, duration_ms = 20, callback = frame(4, false, "walk") },
				{ model = { image = walk_frames["3"] }, duration_ms = 20, callback = frame(3, false, "walk") },
				{ model = { image = walk_frames["2"] }, duration_ms = 20, callback = frame(2, false, "walk") },
				{ model = { image = walk_frames["1"] }, duration_ms = 20, callback = frame(1, false, "walk") },
				
				{ model = { image = walk_frames["1"] }, duration_ms = 20, callback = frame(1, true, "walk_cw") },
				{ model = { image = walk_frames["2"] }, duration_ms = 20, callback = frame(2, true, "walk_cw") },
				{ model = { image = walk_frames["3"] }, duration_ms = 20, callback = frame(3, true, "walk_cw") },
				{ model = { image = walk_frames["4"] }, duration_ms = 20, callback = frame(4, true, "walk_cw") },
				{ model = { image = walk_frames["5"] }, duration_ms = 20, callback = frame(5, true, "walk_cw") },
				{ model = { image = walk_frames["4"] }, duration_ms = 20, callback = frame(4, true, "walk_cw") },
				{ model = { image = walk_frames["3"] }, duration_ms = 20, callback = frame(3, true, "walk_cw") },
				{ model = { image = walk_frames["2"] }, duration_ms = 20, callback = frame(2, true, "walk_cw") },
				{ model = { image = walk_frames["1"] }, duration_ms = 20, callback = frame(1, true, "walk_cw") }
			},
			loop_mode = animation.REPEAT
		}
	
		local new_swing_cw_animation = create_animation {
			frames = {
				{ model = { image = shoot_frames["1"] }, duration_ms = 20, callback = frame(1, true, "swing_cw")  },
				{ model = { image = shoot_frames["2"] }, duration_ms = 20, callback = frame(2, true, "swing_cw")  },
				{ model = { image = shoot_frames["3"] }, duration_ms = 20, callback = frame(3, true, "swing_cw")  },
				{ model = { image = shoot_frames["4"] }, duration_ms = 20, callback = frame(4, true, "swing_cw")  },
				{ model = { image = shoot_frames["5"] }, duration_ms = 65, callback = frame(5, true, "swing_cw")  },
				{ model = { image = shoot_frames["4"] }, duration_ms = 25, callback = frame(4, true, "swing_cw")  },
				{ model = { image = shoot_frames["3"] }, duration_ms = 25, callback = frame(3, true, "swing_cw")  },
				{ model = { image = shoot_frames["2"] }, duration_ms = 25, callback = frame(2, true, "swing_cw")  },
				{ model = { image = shoot_frames["1"] }, duration_ms = 25, callback = frame(1, true, "swing_cw")  }
			},
		
			loop_mode = animation.NONE
		}
		
		local new_swing_ccw_animation = create_animation {
			frames = {
				{ model = { image = shoot_frames["1"] }, duration_ms = 20, callback = frame(1, false, "swing") },
				{ model = { image = shoot_frames["2"] }, duration_ms = 20, callback = frame(2, false, "swing") },
				{ model = { image = shoot_frames["3"] }, duration_ms = 20, callback = frame(3, false, "swing") },
				{ model = { image = shoot_frames["4"] }, duration_ms = 20, callback = frame(4, false, "swing") },
				{ model = { image = shoot_frames["5"] }, duration_ms = 65, callback = frame(5, false, "swing") },
				{ model = { image = shoot_frames["4"] }, duration_ms = 25, callback = frame(4, false, "swing") },
				{ model = { image = shoot_frames["3"] }, duration_ms = 25, callback = frame(3, false, "swing") },
				{ model = { image = shoot_frames["2"] }, duration_ms = 25, callback = frame(2, false, "swing") },
				{ model = { image = shoot_frames["1"] }, duration_ms = 25, callback = frame(1, false, "swing") }
			},
		
			loop_mode = animation.NONE
		}
		
		table.insert(animation_container, new_walk_animation)
		table.insert(animation_container, new_swing_cw_animation)
		table.insert(animation_container, new_swing_ccw_animation)
		
		animation_set = {
			{ event = animation_events.MOVE, animation_response = new_walk_animation },
			{ event = animation_events.MOVE_CW, animation_response = new_walk_animation },
			{ event = animation_events.MOVE_CCW, animation_response = new_walk_animation },
			{ event = animation_events.SWING_CW, animation_response = new_swing_cw_animation },
			{ event = animation_events.SWING_CCW, animation_response = new_swing_ccw_animation }
		}
	elseif weapon_type == "pistol" then
	
	elseif weapon_type == "rifle" then
		local new_walk_animation = create_animation {
			frames = {
				{ model = { image = walk_frames["1"], rotation_offset = 30 }, duration_ms = 30, callback = frame(1, false, "walk") },
				{ model = { image = walk_frames["2"], rotation_offset = 30 }, duration_ms = 30, callback = frame(2, false, "walk") },
				{ model = { image = walk_frames["3"], rotation_offset = 30 }, duration_ms = 30, callback = frame(3, false, "walk") },
				{ model = { image = walk_frames["4"], rotation_offset = 30 }, duration_ms = 30, callback = frame(4, false, "walk") },
				{ model = { image = walk_frames["5"], rotation_offset = 30 }, duration_ms = 30, callback = frame(5, false, "walk") },
				{ model = { image = walk_frames["4"], rotation_offset = 30 }, duration_ms = 30, callback = frame(4, false, "walk") },
				{ model = { image = walk_frames["3"], rotation_offset = 30 }, duration_ms = 30, callback = frame(3, false, "walk") },
				{ model = { image = walk_frames["2"], rotation_offset = 30 }, duration_ms = 30, callback = frame(2, false, "walk") },
				{ model = { image = walk_frames["1"], rotation_offset = 30 }, duration_ms = 30, callback = frame(1, false, "walk") },
			},
			loop_mode = animation.REPEAT
		}
		
		local new_shot_animation = create_animation {
			frames = {
				{ model = { image = shoot_frames["1"], rotation_offset = 30 }, duration_ms = 20, callback = frame(1, false, "shot") },
				{ model = { image = shoot_frames["2"], rotation_offset = 30 }, duration_ms = 20, callback = frame(2, false, "shot") },
				{ model = { image = shoot_frames["3"], rotation_offset = 30 }, duration_ms = 20, callback = frame(3, false, "shot") },
				{ model = { image = shoot_frames["4"], rotation_offset = 30 }, duration_ms = 20, callback = frame(4, false, "shot") },
				{ model = { image = shoot_frames["5"], rotation_offset = 30 }, duration_ms = 20, callback = frame(5, false, "shot") },
				{ model = { image = shoot_frames["4"], rotation_offset = 30 }, duration_ms = 20, callback = frame(4, false, "shot") },
				{ model = { image = shoot_frames["3"], rotation_offset = 30 }, duration_ms = 20, callback = frame(3, false, "shot") },
				{ model = { image = shoot_frames["2"], rotation_offset = 30 }, duration_ms = 20, callback = frame(2, false, "shot") },
				{ model = { image = shoot_frames["1"], rotation_offset = 30 }, duration_ms = 20, callback = frame(1, false, "shot") },
			},
			loop_mode = animation.NONE
		}
		
		table.insert(animation_container, new_walk_animation)
		table.insert(animation_container, new_shot_animation)
		
		animation_set = {
			{ event = animation_events.MOVE, animation_response = new_walk_animation },
			{ event = animation_events.MOVE_CW, animation_response = new_walk_animation },
			{ event = animation_events.MOVE_CCW, animation_response = new_walk_animation },
			{ event = animation_events.SHOT, animation_response = new_shot_animation },
			{ event = animation_events.SWING_CW, animation_response = new_shot_animation },
			{ event = animation_events.SWING_CCW, animation_response = new_shot_animation }
		}
	elseif weapon_type == "melee" then
		local new_walk_cw_animation = create_animation {
			frames = {
				{ model = { image = walk_frames["1"] }, duration_ms = 30, callback = frame(1, true, "walk_cw") },
				{ model = { image = walk_frames["2"] }, duration_ms = 30, callback = frame(2, true, "walk_cw") },
				{ model = { image = walk_frames["3"] }, duration_ms = 30, callback = frame(3, true, "walk_cw") },
				{ model = { image = walk_frames["4"] }, duration_ms = 30, callback = frame(4, true, "walk_cw") },
				{ model = { image = walk_frames["5"] }, duration_ms = 30, callback = frame(5, true, "walk_cw") },
				{ model = { image = walk_frames["4"] }, duration_ms = 30, callback = frame(4, true, "walk_cw") },
				{ model = { image = walk_frames["3"] }, duration_ms = 30, callback = frame(3, true, "walk_cw") },
				{ model = { image = walk_frames["2"] }, duration_ms = 30, callback = frame(2, true, "walk_cw") },
				{ model = { image = walk_frames["1"] }, duration_ms = 30, callback = frame(1, true, "walk_cw") },
			},
			loop_mode = animation.REPEAT
		}
		
		local new_walk_ccw_animation = create_animation {
			frames = {
				{ model = { image = walk_frames["1"] }, duration_ms = 30, callback = frame(1, false, "walk") },
				{ model = { image = walk_frames["2"] }, duration_ms = 30, callback = frame(2, false, "walk")  },
				{ model = { image = walk_frames["3"] }, duration_ms = 30, callback = frame(3, false, "walk")  },
				{ model = { image = walk_frames["4"] }, duration_ms = 30, callback = frame(4, false, "walk")  },
				{ model = { image = walk_frames["5"] }, duration_ms = 30, callback = frame(5, false, "walk")  },
				{ model = { image = walk_frames["4"] }, duration_ms = 30, callback = frame(4, false, "walk")  },
				{ model = { image = walk_frames["3"] }, duration_ms = 30, callback = frame(3, false, "walk")  },
				{ model = { image = walk_frames["2"] }, duration_ms = 30, callback = frame(2, false, "walk")  },
				{ model = { image = walk_frames["1"] }, duration_ms = 30, callback = frame(1, false, "walk")  },
			},
			loop_mode = animation.REPEAT
		}
			
		local new_swing_cw_animation = create_animation {
			frames = {
				{ model = { image = shoot_frames["1"] }, duration_ms = 20, callback = frame(1, true, "swing_cw") },
				{ model = { image = shoot_frames["2"] }, duration_ms = 20, callback = frame(2, true, "swing_cw") },
				{ model = { image = shoot_frames["3"] }, duration_ms = 20, callback = frame(3, true, "swing_cw") },
				{ model = { image = shoot_frames["4"] }, duration_ms = 20, callback = frame(4, true, "swing_cw") },
				{ model = { image = shoot_frames["5"] }, duration_ms = 20, callback = frame(5, true, "swing_cw") }
			},
		
			loop_mode = animation.NONE
		}
		
		local new_swing_ccw_animation = create_animation {
			frames = {
				{ model = { image = shoot_frames["1"] }, duration_ms = 20, callback = frame(1, false, "swing") },
				{ model = { image = shoot_frames["2"] }, duration_ms = 20, callback = frame(2, false, "swing") },
				{ model = { image = shoot_frames["3"] }, duration_ms = 20, callback = frame(3, false, "swing") },
				{ model = { image = shoot_frames["4"] }, duration_ms = 20, callback = frame(4, false, "swing") },
				{ model = { image = shoot_frames["5"] }, duration_ms = 20, callback = frame(5, false, "swing") }
			},
		
			loop_mode = animation.NONE
		}
		
		table.insert(animation_container, new_walk_cw_animation)
		table.insert(animation_container, new_walk_ccw_animation)
		table.insert(animation_container, new_swing_cw_animation)
		table.insert(animation_container, new_swing_ccw_animation)
		
		animation_set = {
			{ event = animation_events.MOVE_CW, animation_response = new_walk_cw_animation },
			{ event = animation_events.MOVE_CCW, animation_response = new_walk_ccw_animation },
			{ event = animation_events.SWING_CW, animation_response = new_swing_cw_animation },
			{ event = animation_events.SWING_CCW, animation_response = new_swing_ccw_animation }
		}
		
	end
	
	return { set = create_animation_set { animations = animation_set }, animations = animation_container } 
end