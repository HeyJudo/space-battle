// strings.h - every piece of text the player sees, in one place.
// Change the wording here and every screen picks it up.
// "%d" marks where a number goes (filled in with TextFormat).
#pragma once

// ---------- Title ----------
const char* const TXT_BOOT_1 = "> BOOT SEQUENCE ........... OK";
const char* const TXT_BOOT_2 = "> WEAPONS ARRAY ........... ONLINE";
const char* const TXT_BOOT_3 = "> HOSTILE CONTACT ......... DETECTED";
const char* const TXT_TITLE_1 = "SPACE";
const char* const TXT_TITLE_2 = "BATTLE";
const char* const TXT_TAGLINE = "TURN-BASED SHIP COMBAT";
const char* const TXT_MENU = "COMMAND";
const char* const TXT_MENU_RIGHT = "SELECT ONE";
const char* const TXT_START = "START BATTLE";
const char* const TXT_START_HINT = "Engage the Warden";
const char* const TXT_HOWTO = "HOW TO PLAY";
const char* const TXT_HOWTO_HINT = "Four moves, one goal";
const char* const TXT_SETTINGS = "SETTINGS";
const char* const TXT_SETTINGS_HINT = "Sound and music";
const char* const TXT_CONTACT_1 = "CONTACT K-7";
const char* const TXT_CONTACT_2 = "RANGE 2.4 KM";
const char* const TXT_FOOTER_LEFT = "HALBERD // ALL SYSTEMS NOMINAL";
const char* const TXT_FOOTER_RIGHT = "SOUND STARTS ON START";

// ---------- Battle ----------
const char* const TXT_HALBERD = "HALBERD";
const char* const TXT_WARDEN = "WARDEN";
const char* const TXT_HULL = "HULL";
const char* const TXT_ENGAGEMENT = "ENGAGEMENT";
const char* const TXT_PHASE_ORDERS = "AWAITING ORDERS";
const char* const TXT_PHASE_TARGETING = "HOSTILE TARGETING";
const char* const TXT_PHASE_INCOMING = "INCOMING";
const char* const TXT_PHASE_OVER = "STAND DOWN";
const char* const TXT_FIRE = "FIRE";
const char* const TXT_FIRE_HINT = "10 to 25 damage";
const char* const TXT_SHIELDS = "SHIELDS";
const char* const TXT_SHIELDS_HINT = "Halve the next hit";
const char* const TXT_REPAIR = "REPAIR";
const char* const TXT_REPAIR_HINT = "Restore 15 to 30 hull";
const char* const TXT_RETREAT = "RETREAT";
const char* const TXT_RETREAT_HINT = "Leave the battle";
const char* const TXT_FEED = "TACTICAL FEED";
const char* const TXT_SHIELDED = "SHIELDED";

// Retreat confirmation
const char* const TXT_CONFIRM_HEADER = "NAV 04";
const char* const TXT_CONFIRM_RIGHT = "CONFIRM";
const char* const TXT_CONFIRM_TITLE = "ABANDON THE FIGHT?";
const char* const TXT_CONFIRM_TEXT = "The Warden will hold the sector if you leave now.";
const char* const TXT_STAY = "STAY";

// Tactical feed lines (the approved Red Alert voice). Each gets an [NN] prefix.
const char* const FEED_START = "RED ALERT. Hostile Warden closing.";
const char* const FEED_FIRE = "WEAPONS FREE. %d DMG CONFIRMED ON TARGET.";
const char* const FEED_SHIELDS = "SHIELDS TO FULL. Next hit will be halved.";
const char* const FEED_REPAIR = "DAMAGE CONTROL. Hull patched +%d.";
const char* const FEED_WARDEN_FIRE = "WE ARE HIT. %d DMG TO HULL.";
const char* const FEED_SHIELDS_HELD = "Shields held. Damage halved.";
const char* const FEED_WARDEN_SHIELDS = "Warden raising shields. Next shot halved.";
const char* const FEED_WARDEN_SHIELDS_HELD = "Warden shields held. Our damage halved.";
const char* const FEED_WARDEN_REPAIR = "Warden running repairs. Hull +%d.";
const char* const FEED_RETREAT = "Helm, full reverse. You left the battle.";

// ---------- How to Play ----------
const char* const TXT_HOWTO_RIGHT = "ONE MOVE PER TURN";
const char* const TXT_HOWTO_FIRE = "Hit the Warden for 10 to 25 damage.";
const char* const TXT_HOWTO_SHIELDS = "The next hit you take is cut in half.";
const char* const TXT_HOWTO_REPAIR = "Restore 15 to 30 hull, up to 100.";
const char* const TXT_HOWTO_RETREAT = "Leave the battle. You will be asked first.";
const char* const TXT_ENEMY = "ENEMY";
const char* const TXT_ENEMY_LINE_1 = "Each turn the Warden fires, raises shields or repairs. Its shields halve";
const char* const TXT_ENEMY_LINE_2 = "your next shot.";
const char* const TXT_GOAL = "GOAL";
const char* const TXT_GOAL_LINE = "Bring the Warden's hull to 0 before yours runs out.";
const char* const TXT_GOT_IT = "GOT IT";

// ---------- Settings ----------
const char* const TXT_SETTINGS_RIGHT = "AUDIO";
const char* const TXT_SOUND_EFFECTS = "SOUND EFFECTS";
const char* const TXT_MUSIC = "MUSIC";
const char* const TXT_REDUCE_MOTION = "REDUCE MOTION";
const char* const TXT_REDUCE_MOTION_HINT = "No shakes, flicker or drifting stars";
const char* const TXT_ON = "ON";
const char* const TXT_OFF = "OFF";
const char* const TXT_APPLY = "CHANGES APPLY RIGHT AWAY";
const char* const TXT_BACK = "BACK";

// ---------- Victory and Defeat ----------
const char* const TXT_VICTORY = "VICTORY";
const char* const TXT_VICTORY_LINE = "> WARDEN DESTROYED. SECTOR CLEAR.";
const char* const TXT_DEFEAT = "DEFEAT";
const char* const TXT_DEFEAT_LINE = "> HULL BREACH. HALBERD IS LOST.";
const char* const TXT_DEBRIEF = "DEBRIEF";
const char* const TXT_DEBRIEF_RIGHT = "AFTER ACTION";
const char* const TXT_TURNS = "TURNS";
const char* const TXT_DAMAGE_DEALT = "DAMAGE DEALT";
const char* const TXT_HULL_LEFT = "HULL LEFT";
const char* const TXT_WARDEN_HULL = "WARDEN HULL";
const char* const TXT_PLAY_AGAIN = "PLAY AGAIN";
const char* const TXT_TRY_AGAIN = "TRY AGAIN";
const char* const TXT_MAIN_MENU = "MAIN MENU";

// ---------- Phone held upright ----------
const char* const TXT_ROTATE = "ROTATE YOUR PHONE";
const char* const TXT_ROTATE_HINT = "Space Battle plays in landscape";
