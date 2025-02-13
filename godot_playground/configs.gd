class_name Configs

extends Object

static var hp_scale: int = 1500
static var move_speed_scale: float = 3.0
static var attack_speed_scale: float = 1.0
static var attack_damage_scale: int = 120
static var attack_range_scale: float = 1.2

class FightConfig:
	enum Mode {
		DEMO,
		AUTOCHESS
	}
	var characters_configs: Array[CharacterConfig]
	var mode: Mode
	var max_characters_in_team: int
	
	func _init(
		characters_configs: Array[CharacterConfig],
		mode: Mode,
		max_characters_in_team: int
	):
		self.characters_configs = characters_configs
		self.mode = mode
		self.max_characters_in_team = max_characters_in_team


class CharacterConfig:
	enum CharacterClass { 
		WARRIOR, MAGE, RIFLEMAN, TANK
	}
	var name: String
	var scene_path: String
	var cls: CharacterClass
	
	var hp: float
	var move_speed: float
	var attack_damage: float
	var attack_range: float
	var attack_speed: float
	
	var run_anim: String
	var fight_anims: Array[String]
	var stance_anims: Array[String]
	var abilities: Array[AbilityConfig]
	
	var level_up: LevelUpConfig
	
	static func from_json(character_config: Variant) -> CharacterConfig:
		var cfg = CharacterConfig.new()
		match character_config.class:
			"warrior": cfg.class = CharacterClass.WARRIOR
			"mage": cfg.class = CharacterClass.MAGE
			"rifleman": cfg.class = CharacterClass.RIFLEMAN
			"tank": cfg.class = CharacterClass.TANK
			var unknown: assert(false, "Unknown character class %s" % unknown)
		
		cfg.name = character_config.name
		cfg.scene_path = character_config.scene_path
		cfg.hp = character_config.hp * Configs.hp_scale
		cfg.move_speed = character_config.move_speed * Configs.move_speed_scale
		cfg.attack_damage = character_config.attack_damage * Configs.attack_damage_scale
		cfg.attack_range = character_config.attack_range * Configs.attack_range_scale
		cfg.attack_speed = character_config.attack_speed * Configs.attack_speed_scale
		
		cfg.run_anim = character_config.run_anim
		cfg.fight_anims = character_config.fight_anims
		cfg.stance_anims = character_config.stance_anims
		cfg.abilities = character_config.abilities.map(AbilityConfig.from_json)
		
		cfg.level_up = LevelUpConfig.from_json(character_config.level_up)
		return cfg

class LevelUpConfig:
	var hp_update: float
	var move_speed_update: float
	var attack_damage_update: float
	var attack_range_update: float
	var attack_speed_update: float
	
	static func from_json(cfg: Variant) -> LevelUpConfig:
		var obj = LevelUpConfig.new()
		obj.hp_update = cfg.hp_update * Configs.hp_scale
		obj.move_speed_update = cfg.move_speed_update * Configs.hp_scale
		obj.attack_damage_update = cfg.attack_damage_update * Configs.attack_damage_scale
		obj.attack_range_update = cfg.attack_range_update * Configs.attack_range_scale
		obj.attack_speed_update = cfg.attack_speed_update * Configs.attack_speed_scale

		return obj

class AbilityConfig:
	enum Type { 
		TARGET_PROJECTILE,
		AOE_PROJECTILE,
		BUFF
	}
	var type: Type
	var cooldown: float # in seconds
	var cast_duration: float # in seconds
	var is_spelling_interruptable: bool
	var wiped_on_death: bool
	const is_exclusive: bool = true # Just a marker to think about it later
	const scene_name: String = "" # Just a marker to think about it later
	
	static func from_json(cfg: Variant) -> AbilityConfig:
		# TODO: Implement later
		return AbilityConfig.new()
