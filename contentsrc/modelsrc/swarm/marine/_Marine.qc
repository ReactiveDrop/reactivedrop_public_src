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
$addsearchdir dmx/animation/
$eyeposition 0 0 70
$body body "Marine.dmx"

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
	blank
}

$surfaceprop "flesh"
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
$include "_DefineBones.qci"
$include "_HierarchyChainsWeights.qci"
$include "_BoneMerges.qci"

$includemodel swarm/marine/marine_anims.mdl
$includemodel swarm/marine/marine_anims_extra.mdl

$include "_Attachments.qci"
$include "_Hitbox.qci"
$include "_Ragdoll.qci"
$include "_JiggleBones.qci"
