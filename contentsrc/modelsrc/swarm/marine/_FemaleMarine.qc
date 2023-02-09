//==============================================================================
// Female Marine
//==============================================================================





//==============================================================================
// the model + basics
//==============================================================================
$modelname swarm\marine\FemaleMarine.mdl
$cdmaterials player\male
$addsearchdir dmx\
$eyeposition 0 0 70
$scale 0.88
$model MarineTest "Marine.dmx" {
	mouth 0 "mouth" "ValveBiped.Bip01_Head1" 0 1 0     // mouth illumination
}

$bodygroup back_units
{
	blank
	studio "dmx/officer_back_unit.dmx"
	studio "dmx/medic_back_unit.dmx"
	studio "dmx/special_weapons_back_unit.dmx"
	studio "dmx/tech_back_unit.dmx"
}



$surfaceprop "flesh"
//$bbox -13 -13 0  13 13 72
//$bbox -100 -100 0  100 100 150
// final:
//$bbox -8 -8 0  8 8 72

$texturegroup skinfamilies 
{

	{ "medic_body" "medic_helmet" }
	{ "models/player/male/officer_body" "models/player/male/officer_helmet" }
	{ "models/player/male/medic_body_female" "models/player/male/medic_helmet_female" }
	{ "models/player/male/special_weapons_body_female" "models/player/male/special_weapons_helmet_female" }
	{ "models/player/male/tech_body" "models/player/male/tech_helmet" }
}

//==============================================================================
// attachments
//==============================================================================
$attachment "mouth" "ValveBiped.Bip01_Head1" 1.00 -6.30 0.00 rotate 0 -80 -90
$attachment "Center" "ValveBiped.Bip01_Spine1" -6.20 2.09 0.00 rotate -0 97 90
$attachment "LHand" "ValveBiped.Bip01_L_Hand" 3.00 -0.00 -0.00 rotate -0 0 -0
$attachment "RHand" "ValveBiped.Bip01_R_Hand" 3.00 0.00 0.00 rotate 0 -0 -0
$attachment "anim_attachment_head" "ValveBiped.Bip01_Head1" -0.00 0.00 0.00 rotate -90 -90 0
$attachment "baton" "ValveBiped.baton_parent" 0.00 -0.00 0.00 rotate -0 -0 -0
$attachment "manhack" "ValveBiped.Bip01_Spine" 18.00 -6.00 3.00 rotate -25 50 0
$attachment "lefthand" "ValveBiped.Bip01_L_Hand" 0.00 -0.00 -0.00 rotate -0 0 -0
$attachment "zipline" "ValveBiped.Bip01_Spine4" 3.00 -4.00 -0.00 rotate 0 -0 -0
$attachment "eyes" "ValveBiped.Bip01_Head1" 4.50 -5.00 0.00 rotate 0 -90 -90
$attachment "head" "ValveBiped.Bip01_Head1" 0.00 0.00 0.00 rotate 0 -80 -90
$attachment "chest" "ValveBiped.Bip01_Spine4" -3.00 6.00 0.00 rotate 0 60 90
$attachment "chestbleed" "ValveBiped.Bip01_Spine4" -3.00 13.00 0.00 rotate 0 60 90
$attachment "forward" "ValveBiped.Bip01_Spine4" -3.00 6.00 0.00 rotate 0 60 90
$attachment "shoulderlight" "ValveBiped.Bip01_Head1" 3.50 1.50 -8.00 rotate 5 -95 -90
$attachment "beam_attach" "ValveBiped.Bip01_Spine1" 0 0 0 rotate 0 0 0
$attachment "melee_left_hand" "ValveBiped.Bip01_L_Hand" 10.00 -5.00 -5.00 rotate -0 0 -0
$attachment "melee_right_hand" "ValveBiped.Bip01_R_Hand" 10.00 -5.00 -5.00 rotate 0 -0 -0
$attachment "melee_left_foot" "ValveBiped.bip01_L_Foot" 3.00 -0.00 -0.00 rotate -0 0 -0
$attachment "melee_right_foot" "ValveBiped.bip01_R_Foot" 3.00 -0.00 -0.00 rotate -0 0 -0
$attachment "backpack" "ValveBiped.Bip01_Spine4" 6.00 -10.00 -0.00 rotate 0 -0 -0
$attachment "jump_jet_l" "ValveBiped.Bip01_Spine4" -8.00 -10.00 5.00 rotate 0 -0 -0
$attachment "jump_jet_r" "ValveBiped.Bip01_Spine4" -8.00 -10.00 -5.00 rotate 0 -0 -0
$bonemerge "ValveBiped.Bip01_R_Hand"
$bonemerge "ValveBiped.Bip01_L_Hand"
$bonemerge "ValveBiped.Bip01_Spine2"

//==============================================================================
// includes
//==============================================================================
$include "_HierarchyChainsWeights.qci"

// idles/walk/run
$include "_BaseAnims.qci"

// stop stand transition anim being inserted between these nodes
$skiptransition "running" "walking"

$include "_WeaponAnims.qci"
$include "_FlinchAnims.qci"
$include "_Hitbox.qci"
$include "_Ragdoll.qci"