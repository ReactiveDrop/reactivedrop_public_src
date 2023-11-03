$modelname "sentry_gun/railgun_top.mdl"

$body "base" "sentry_top_base.dmx"
$body "railgun_top" "railgun_top.dmx"

$surfaceprop "weapon"
$contents "solid"

$cdmaterials "swarm/models/SentryGun"
$cdmaterials "models/weapons/mininglaser"
$cdmaterials "models/weapons/Railgun"

$attachment "muzzle" "ValveBiped.sentrygun_Barrel" 0 0 0 rotate 0 0 90
$attachment "atttach_camera" "ValveBiped.sentrygun_camera_lense2" 0 0 0 rotate 0 0 -90
$attachment "eject1" "ValveBiped.sentrygun_eject1" 0 0 0 rotate 0 0 -90
$attachment "attach_light" "ValveBiped.sentrygun_screen" -2 0 -2 rotate 180 90 0

$animation "aim_up" "railgun_top_anims/aim_up.smd"
$animation "aim_mid" "railgun_top_anims/aim_mid.smd"
$animation "aim_down" "railgun_top_anims/aim_down.smd"
$animation "camera_aim_base" "sentry_anims/camera_aim.dmx" {
	frame 1 1
}
$animation "camera_aim_left" "sentry_anims/camera_aim.dmx" {
	frame 0 0
	subtract "camera_aim_base" 0
}
$animation "camera_aim_mid" "sentry_anims/camera_aim.dmx" {
	frame 1 1
	subtract "camera_aim_base" 0
}
$animation "camera_aim_right" "sentry_anims/camera_aim.dmx" {
	frame 2 2
	subtract "camera_aim_base" 0
}
$animation "disassemble" "railgun_top_anims/disassemble.smd"

$sequence "bindpose" {
	"sentry_anims/bindpose.dmx"
	activity ACT_OBJ_IDLE 1
}

$sequence "sentry_fire" {
	"railgun_top_anims/turret_fire.smd"
	activity ACT_OBJ_RUNNING 1
	rotate -90
	delta
}

$sequence "sentry_build" {
	"sentry_anims/railgun_sentry_build.dmx"
	activity ACT_OBJ_ASSEMBLING 1
}

$sequence "sentry_updown" {
	"aim_up" "aim_mid" "aim_down"
	autoplay
	blend "aim_pitch" 45 -45
	blendwidth 3
	delta
	hidden
}

$sequence "sentry_camera" {
	"camera_aim_left" "camera_aim_mid" "camera_aim_right"
	autoplay
	blend "aim_yaw" 120 -120
	blendwidth 3
	delta
	hidden
}

$sequence "sentry_disassemble" {
	"aim_mid" "disassemble"
	autoplay
	blend "aim_pitch" -45 -90
	blendwidth 2
	delta
	hidden
}
