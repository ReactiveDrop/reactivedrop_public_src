// "NPC" {
//     "AlienClass"    "asw_drone_uber"
//     "VScript"       "drone_mutation_carrier"
// }

parasiteProp <- CreateProp("prop_dynamic", self.GetOrigin(), "models/aliens/parasite/parasite.mdl", 17);

function CarrierDroneDied() {
	realParasite <- Director.SpawnAlienAt("asw_parasite", parasiteProp.GetOrigin(), parasiteProp.GetAngles());
	parasiteProp.Destroy();

	self.DisconnectOutput("OnDeath", "CarrierDroneDied");
	self.DisconnectOutput("OnIgnite", "CarrierDroneIgnited");
}

function CarrierDroneIgnited() {
	function ParasiteJumpAttack() {
		self.JumpAttack();
		self.DisconnectOutput("OnIgnite", "ParasiteJumpAttack");
	}

	realParasite <- Director.SpawnAlienAt("asw_parasite", parasiteProp.GetOrigin(), parasiteProp.GetAngles());
	realParasite.ValidateScriptScope();
	realParasite.GetScriptScope().ParasiteJumpAttack <- ParasiteJumpAttack;
	realParasite.ConnectOutput("OnIgnite", "ParasiteJumpAttack");
	EntFireByHandle(parasiteProp, "Ignite", "", 0, activator, caller);
	parasiteProp.Destroy();

	self.DisconnectOutput("OnDeath", "CarrierDroneDied");
	self.DisconnectOutput("OnIgnite", "CarrierDroneIgnited");
}

EntFireByHandle(self, "Color", "250 103 5", 0, self, self);
parasiteProp.SetOwner(self);

function RotateToFaceForward() {
	self.SetLocalAngles(0, 90, 0);
	self.DisconnectOutput("OnUser1", "RotateToFaceForward");
}
parasiteProp.ValidateScriptScope();
parasiteProp.GetScriptScope().RotateToFaceForward <- RotateToFaceForward;

EntFireByHandle(parasiteProp, "SetDefaultAnimation", "ragdoll", 0, self, self);
EntFireByHandle(parasiteProp, "SetAnimation", "ragdoll", 0, self, self);
EntFireByHandle(parasiteProp, "DisableShadow", "", 0, self, self);
EntFireByHandle(parasiteProp, "SetParent", "!activator", 0, self, self);
EntFireByHandle(parasiteProp, "SetParentAttachment", "blood_spray", 0, self, self);
parasiteProp.ConnectOutput("OnUser1", "RotateToFaceForward");
EntFireByHandle(parasiteProp, "FireUser1", "", 0, self, self);

self.ValidateScriptScope();
self.GetScriptScope().CarrierDroneDied <- CarrierDroneDied;
self.ConnectOutput("OnDeath", "CarrierDroneDied");
self.GetScriptScope().CarrierDroneIgnited <- CarrierDroneIgnited;
self.ConnectOutput("OnIgnite", "CarrierDroneIgnited");
