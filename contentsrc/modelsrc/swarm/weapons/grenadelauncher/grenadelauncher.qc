$modelname "weapons/GrenadeLauncher/GrenadeLauncher.mdl"

$body "default" "grenadelauncher_model.smd"

$bodygroup "magazine"
{
	studio "grenadelauncher_drum_in.smd"
	studio "grenadelauncher_drum_out.smd"
}
$bodygroup "grip"
{
	studio "grenadelauncher_grip.smd"
	studio "grenadelauncher_railgun_grip.smd"
	blank
}
$bodygroup "scope"
{
	blank
	studio "grenadelauncher_scope.smd"
}
$bodygroup "sight"
{
	studio "grenadelauncher_rearsight_down.smd"
	studio "grenadelauncher_rearsight_up.smd"
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/GrenadeLauncher"
$cdmaterials "swarm/models/ASDecorationSkins"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "grenadelauncher_model.smd"

$collisionmodel "grenadelauncher_physics.smd"
