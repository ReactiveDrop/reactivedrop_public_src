$modelname "weapons/HealGun/HealGun.mdl"

$body "default" "healgun_model.smd"

$bodygroup "magazine"
{
	studio "healgun_cannister.smd"
	blank
}
$bodygroup "screen"
{
	studio "healgun_screen.smd"
	studio "healgun_screen_off.smd"
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/HealGun"

$texturegroup "skinfamilies"
{
	{ "HealGun" }
	{ "HealAmpGun" }
}

$attachment "muzzle" "ValveBiped.Bip01_R_Hand" 19 -2.5 -0.5 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "healgun_model.smd"

$collisionmodel "healgun_physics.smd"
