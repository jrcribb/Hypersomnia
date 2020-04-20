## Editor GUI

- General synchronization between nodes and actual entities on scene
	- cosmic entity data -> proxy nodes
		- The only functionality of proxy nodes over the entities will be hierarchicization of transforms and prefabization
	- Or... only ever proxy nodes -> cosmic entity data ?
		- Sibling order -> render layer index (int)
			- we'll want some layers to be always above others though. Like crates above the floor.
	- Some reinference procedure for the proxies
		- on rewrite_change in move entities command just recalculate the transforms of entities attached to descendant nodes
		- yeah, we'll keep track of all entity to node mappings

- .arena file has settings and the path to root node

- Unpacking .arena file (relevant when thinking about the proxy node structure)
	- Recursively from root node
		- Just unprefabize and set transforms in accordance with the hierarchy
			- Though the cosmos should stay in sync at all times
			- So there won't really be much unpacking to do. Only during load once to memory and to create the cache.
			- on save we'll just dump the proxy cosmos to .solvable and flavours to .common

	

- Only one project will be open
	- To avoid confusion and overly complex architecture
	- We might want to auto-save only the opened and unsaved nodes (like text files)
		- as well as camera view
		- Well, other common stuff too


- Layout tool
	- Biomes

- Perhaps an easy layout builder?
	- Problematically, we have to lay out the walls somehow automatically?


- Left-down corner dock
	- Filesystem
		- Project (default)
		- Official
			- From here you can drag and drop stuff like aquarium and others
			- BEGIN BY DRAG AND DROPPING SOME PREFABS TO THE SCREEN
		- what about other maps? We might want to use existing collections
			- what if they later get removed from these maps?