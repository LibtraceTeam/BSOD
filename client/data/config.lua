levels = {
	"data/bung.barc/levels/q3/q3tourney1.blev",		-- 1 lotsa beziers
	"data/bung.barc/levels/q3/q3tourney2.blev",		-- 2 small-mid
	"data/bung.barc/levels/q3/q3tourney5.blev",		-- 3 small and square
	"data/bung.barc/levels/q3/q3ctf1.blev"	,		-- 4 mid size
	"data/bung.barc/levels/q3/q3dm11.blev"	,		-- 5 Big!
	"data/bung.barc/levels/q3/q3dm1.blev",			-- 6 small with demon tongue
	"data/bung.barc/levels/q3/atrium.blev",			-- 7
	"data/bung.barc/levels/q3/archives.blev",		-- 8
	"data/bung.barc/levels/q3/countryclub.blev",	-- 9
	"data/bung.barc/levels/q3/bungmap.blev",		-- 10
	"data/bung.barc/levels/q3/newmap.blev",			-- 11
	"data/bung.barc/levels/q3/sam.blev",			-- 12

	"bungmap/level_creation/levels/q3/q3dm1.blev",  -- 13
	"bungmap/level_creation/levels/q3/q3ctf1.blev",  -- 14
	}
	

level_name		= levels[13]

start_location	= {-3.50,1.53,24.0}
pitch = 0.8
heading = -10.3

display			=  "opengl" -- "d3d" -- 
fullscreen		= "no"
width			= 1024
height			= 768 
bpp				= 32

-- Hawk net driver can use "host:port" notation
network_host	= "voodoo:32500"
-- Do not specify port for enet (for now)
--network_host    = "localhost" 
