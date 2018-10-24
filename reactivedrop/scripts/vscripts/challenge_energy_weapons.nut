Convars.SetValue( "rm_destroy_empty_weapon", 0 );
Convars.SetValue( "rd_hud_hide_clips", 1 );

nextRefillTick1 <- {};
nextRefillTick2 <- {};
lastClip1 <- {};
lastClip2 <- {};

timeBetweenRefill1 <- 0.1;
timeBetweenRefill2 <- 10;
timeBetweenInverse1 <- 75;
timeBetweenInverse2 <- 20;
timeAfterShoot1 <- 5;
timeAfterShoot2 <- 25;
sentryEmptyFiringDelay <- 15;
singleClipPenalty <- 0.55;
noClipPenalty <- 4;

ammoEntities <- [
	"asw_ammo_drop",
	"asw_ammo_rifle",
	"asw_ammo_autogun",
	"asw_ammo_shotgun",
	"asw_ammo_vindicator",
	"asw_ammo_flamer",
	"asw_ammo_pistol",
	"asw_ammo_mining_laser",
	"asw_ammo_railgun",
	"asw_ammo_chainsaw",
	"asw_ammo_pdw",
	"asw_weapon_ammo_satchel",
	"asw_pickup_ammo_satchel"
];

function Update() {
	minDelay <- 0.2;

	foreach ( classname in ammoEntities ) {
		ammo <- null;
		while ( ( ammo = Entities.FindByClassname( ammo, classname ) ) != null ) {
			ammo.Destroy();
		}
	}

	marine <- null;
	while ( ( marine = Entities.FindByClassname( marine, "asw_marine" ) ) != null ) {
		marine.RemoveAllAmmo();
		for ( local weapon = marine.FirstMoveChild(); weapon != null; weapon = weapon.NextMovePeer() ) {
			if ( !( "GetMaxClip1" in weapon ) ) {
				continue;
			}

			local maxClip1 = weapon.GetMaxClip1();
			if (maxClip1 <= 0) {
				continue;
			}

			local clips1 = weapon.GetMaxAmmo1() / maxClip1;
			if ( clips1 < noClipPenalty ) {
				maxClip1 = ceil( maxClip1 * ( 1 - singleClipPenalty / clips1 ) );
			}

			if ( weapon.Clip1() > maxClip1 ) {
				weapon.SetClip1( maxClip1 );
			}

			if ( !( weapon.entindex() in nextRefillTick1 ) || !( weapon.entindex() in nextRefillTick2 ) ) {
				nextRefillTick1[weapon.entindex()] <- Time();
				nextRefillTick2[weapon.entindex()] <- Time();
				lastClip1[weapon.entindex()] <- weapon.Clip1();
				lastClip2[weapon.entindex()] <- weapon.Clip2();
				continue;
			}

			if ( weapon.Clip1() != lastClip1[weapon.entindex()] ) {
				lastClip1[weapon.entindex()] = weapon.Clip1();
				nextRefillTick1[weapon.entindex()] = Time() + timeAfterShoot1 + timeBetweenInverse1 / weapon.GetMaxAmmo1();
			}

			if ( nextRefillTick1[weapon.entindex()] <= Time() && weapon.Clip1() < maxClip1 ) {
				weapon.SetClip1( weapon.Clip1() + 1 );
				lastClip1[weapon.entindex()] = weapon.Clip1();
				delay <- timeBetweenRefill1 + timeBetweenInverse1 / weapon.GetMaxAmmo1();
				nextRefillTick1[weapon.entindex()] = Time() + delay;
				if ( delay < minDelay ) {
					minDelay = delay;
				}
			}

			if (weapon.GetMaxClip2() <= 0) {
				continue;
			}

			if ( weapon.Clip2() != lastClip2[weapon.entindex()] ) {
				lastClip2[weapon.entindex()] = weapon.Clip2();
				nextRefillTick2[weapon.entindex()] = Time() + timeAfterShoot2 + timeBetweenInverse2 / weapon.GetMaxAmmo2();
			}

			if ( nextRefillTick2[weapon.entindex()] <= Time() && weapon.Clip2() < weapon.GetDefaultClip2() ) {
				weapon.SetClip2( weapon.Clip2() + 1 );
				lastClip2[weapon.entindex()] = weapon.Clip2();
				delay <- timeBetweenRefill2 + timeBetweenInverse2 / weapon.GetMaxAmmo2();
				nextRefillTick2[weapon.entindex()] = Time() + delay;
				if ( delay < minDelay ) {
					minDelay = delay;
				}
			}
		}
	}

	sentry <- null;
	while ( ( sentry = Entities.FindByClassname( sentry, "asw_sentry_base" ) ) != null ) {
		if ( !( sentry.entindex() in nextRefillTick1 ) ) {
			nextRefillTick1[sentry.entindex()] <- Time();
			lastClip1[sentry.entindex()] <- sentry.GetAmmo();
			continue;
		}

		if ( sentry.GetAmmo() != lastClip1[sentry.entindex()] ) {
			lastClip1[sentry.entindex()] = sentry.GetAmmo();
			nextRefillTick1[sentry.entindex()] = Time() + timeAfterShoot1;
		}

		if ( nextRefillTick1[sentry.entindex()] < Time() && sentry.GetAmmo() < sentry.GetMaxAmmo() ) {
			if ( sentry.GetAmmo() == 0 ) {
				sentry.GetSentryTop().PreventFiringUntil( Time() + sentryEmptyFiringDelay );
			}
			sentry.SetAmmo( sentry.GetAmmo() + 1 );
			lastClip1[sentry.entindex()] = sentry.GetAmmo();
			delay <- timeBetweenRefill1 + timeBetweenInverse1 / sentry.GetMaxAmmo();
			nextRefillTick1[sentry.entindex()] = Time() + delay;
			if ( delay < minDelay ) {
				minDelay = delay;
			}
		}
	}

	return minDelay;
}
