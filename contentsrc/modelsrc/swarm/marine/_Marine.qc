//==============================================================================
// Marine
//==============================================================================





//==============================================================================
// the model + basics
//==============================================================================
$modelname swarm/marine/marine.mdl
$cdmaterials models/player/male
$cdmaterials models/player/male/back_units
$addsearchdir dmx/
$eyeposition 0 0 70
$model body "Marine.dmx" {
	mouth 0 "mouth" "ValveBiped.Bip01_Head1" 0 1 0     // mouth illumination
}

$bodygroup back_units {
	blank
	studio "dmx/officer_back_unit.dmx"
	studio "dmx/medic_back_unit.dmx"
	studio "dmx/special_weapons_back_unit.dmx"
	studio "dmx/tech_back_unit.dmx"
}

$bodygroup helmets
{
	studio "dmx/marine_helmet.dmx"
}

$surfaceprop "armorflesh"
$bbox -13 -13 0  13 13 72

$texturegroup skinfamilies
{
	{ "medic_body" "medic_helmet"}
	{ "officer_body" "officer_helmet"}
	{ "medic_body" "medic_helmet" }
	{ "special_weapons_body" "special_weapons_helmet" }
	{ "tech_body" "tech_helmet" }
}

//==============================================================================
// includes
//==============================================================================
$include "_Attachments.qci"
$include "_HierarchyChainsWeights.qci"

// idles/walk/run
$include "_BaseAnims.qci"

// stop stand transition anim being inserted between these nodes
$skiptransition "running" "walking"

$include "_WeaponAnims.qci"
$include "_FlinchAnims.qci"
$include "_Hitbox.qci"
$include "_Ragdoll.qci"
