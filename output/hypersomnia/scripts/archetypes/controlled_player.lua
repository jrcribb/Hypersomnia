local player_physics_component = {
	body_type = Box2D.b2_dynamicBody,
	
	body_info = {
		filter = filters.CHARACTER,
		shape_type = physics_info.RECT,
		rect_size = vec2(37, 37),
		
		angular_damping = 5,
		--linear_damping = 18,
		--max_speed = 3300,
		
		fixed_rotation = true,
		density = 0.1,
		angled_damping = true
	}
}
			
local player_movement_component = {
	input_acceleration = vec2(10000, 10000),
	max_accel_len = 10000,
	max_speed_animation = 800,
	air_resistance = 0.2,
	braking_damping = 18,
	receivers = {
		{ target = "body", stop_at_zero_movement = false }, 
		{ target = "legs", stop_at_zero_movement = true  }
	}
}

function create_simulation_player(owner_world)
	return owner_world:create_entity {
		transform = {},
		physics = player_physics_component,
		movement = player_movement_component
	}
end

function create_controlled_player(scene_object, position, target_camera, crosshair_sprite)
	local player = scene_object.world_object:create_entity_group  {
		-- body also acts as torso
		body = {
			render = {
				layer = render_layers.PLAYERS,
				model = blank_green
			},
		
			transform = {
				pos = position
			},
			
			animate = {
			
			},
					
			physics = player_physics_component,
			
			input = {
				intent_message.MOVE_FORWARD,
				intent_message.MOVE_BACKWARD,
				intent_message.MOVE_LEFT,
				intent_message.MOVE_RIGHT,
				custom_intents.PICK_REQUEST,
				intent_message.SHOOT
			},
			
			lookat = {
				target = "crosshair",
				look_mode = lookat_component.POSITION
			},
	
			particle_emitter = {
				available_particle_effects = scene_object.particles.npc_effects
			},
			
			movement = player_movement_component,
		 
			children = {
				"legs",
				"crosshair"
			}
		},
	
		crosshair = { 
			transform = {
				pos = vec2(0, 0),
				rotation = 0
			},
			
			render = {
				layer = render_layers.CROSSHAIRS,
				model = crosshair_sprite
			},
			
			crosshair = {
				sensitivity = config_table.sensitivity,
				should_blink = false
			},
			
			chase = {
				target = "body",
				relative = true
			},
			
			input = {
				intent_message.AIM
			}
		},
		
		legs = {
			transform = { 
				pos = position,
				rotation = 0
			},
		
			render = {
				layer = render_layers.LEGS,
				model = nil
			},
		
			chase = {
				target = "body"
			},
		
			lookat = {
				target = "body",
				look_mode = lookat_component.VELOCITY
			},
		
			animate = {
			
			}
		}
	}
	
	if target_camera ~= nil then
		target_camera.chase:set_target(player.body)
		target_camera.camera.player:set(player.body)
		target_camera.camera.crosshair:set(player.crosshair)
	end
	
	player.body.animate.available_animations = scene_object.torso_sets["basic"]["barehands"].set
	player.legs.animate.available_animations = scene_object.legs_sets["basic"].set
	
	return player
end

world_archetype_callbacks.CONTROLLED_PLAYER = {
	creation = function(self, id)
		local player_cpp_entity = create_controlled_player(
		self.owner_scene,
		self.owner_scene.teleport_position, 
		self.owner_scene.world_camera, 
		self.owner_scene.crosshair_sprite)
	
		local new_entity = components.create_components {
			cpp_entity = player_cpp_entity.body,
			input_prediction = {
				simulation_entity = self.owner_scene.simulation_player
			},
			
			orientation = {
				receiver = false,
				crosshair_entity = player_cpp_entity.crosshair
			},
			
			health = {},

			wield = {
				wield_offsets = npc_wield_offsets
			},
			
			label = {
				position = components.label.positioning.OVER_HEALTH_BAR,
				default_font = self.owner_scene.font_by_name["kubasta"],
				
				text = {}
			}
		}
		
		new_entity.wield.on_item_wielded = function(this, picked, old_item, wielding_key)
			if wielding_key == components.wield.keys.PRIMARY_WEAPON then
				player_cpp_entity.body.animate.available_animations = self.owner_scene.torso_sets["basic"][picked.item.outfit_type].set
				
				player_cpp_entity.body.movement.animation_message = animation_events.MOVE
				
				if picked.weapon ~= nil then
					if picked.weapon.is_melee then
						if picked.weapon.current_swing_direction then
							player_cpp_entity.body.movement.animation_message = animation_events.MOVE_CW
						else
							player_cpp_entity.body.movement.animation_message = animation_events.MOVE_CCW
						end
					end
					
					picked.weapon.transmit_bullets = true
					picked.weapon.constrain_requested_bullets = true
					picked.weapon.bullet_entity.physics.body_info.filter = filters.BULLET
				end
				
				
				local stop_msg = animate_message()
				stop_msg.subject = player_cpp_entity.body
				stop_msg.message_type = animate_message.STOP
				stop_msg.animation_priority = 100
				
				local msg = animate_message()
				msg.subject = player_cpp_entity.body
				msg.message_type = animate_message.START
				msg.change_speed = true
				msg.animation_type = player_cpp_entity.body.movement.animation_message
				msg.speed_factor = 0
				msg.animation_priority = 0
				
				player_cpp_entity.body.owner_world:post_message(stop_msg)
				player_cpp_entity.body.owner_world:post_message(msg)
			end
		end
		
		new_entity.wield.on_item_unwielded = function(this, unwielded, wielding_key)
			if wielding_key == components.wield.keys.PRIMARY_WEAPON then
				player_cpp_entity.body.animate.available_animations = self.owner_scene.torso_sets["basic"]["barehands"].set
				player_cpp_entity.body.movement.animation_message = animation_events.MOVE
				
				if unwielded.cpp_entity.physics == nil then return end
				
				local body = unwielded.cpp_entity.physics.body
				local force = (this.orientation.last_pos):normalize() * 100
				
				if this.orientation.last_pos:length() < 0.01 then
					force = vec2(100, 0)
				end
				print "force:" 
				print (force.x, force.y)
				body:ApplyLinearImpulse(to_meters(force), body:GetWorldCenter(), true)
				body:ApplyAngularImpulse(4, true)
				
				local stop_msg = animate_message()
				stop_msg.subject = player_cpp_entity.body
				stop_msg.message_type = animate_message.STOP
				stop_msg.animation_priority = 100
			end
		end
		
		self.controlled_character_id = id
		
		return new_entity
	end
}


