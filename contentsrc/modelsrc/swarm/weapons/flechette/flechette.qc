$modelname "weapons/flechette/flechette.mdl"

$body "default" "flechette_model.smd"

$bodygroup "magazine"
{
	studio "flechette_mag.smd"
	blank
}
$bodygroup "bolt"
{
	studio "flechette_projectile.smd"
	blank
}
$bodygroup "sight"
{
	studio "flechette_sight.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$mostlyopaque

$cdmaterials "models/weapons"
$cdmaterials "models/weapons/flechette"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "flechette_model.smd"

$collisionmodel "flechette_physics.smd"
