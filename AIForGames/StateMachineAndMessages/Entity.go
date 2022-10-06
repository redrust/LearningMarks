package main

type IProperty interface {
	GetID() uint64
	GetPosition() []float64
}

type IBase interface {
	Init()
	Update()
	Destroy()
	OnMsg(uint64, Msg)
}

type Entity struct {
	id        uint64
	position  []float64
	rotation  []float64
	hp        uint32
	mp        uint32
	moveSpeed uint32
}

var globalID uint64 = 1
var defaultPosition = []float64{0, 0, 0}
var defaultRotation = []float64{0, 0, 0}
var defaultHP uint32 = 100
var defaultMP uint32 = 100
var defaultMoveSpeed uint32 = 200
var EntityMap = map[uint64]IBase{}

func (entity *Entity) GetID() uint64 {
	return entity.id
}

func (entity *Entity) GetPosition() []float64 {
	return entity.position
}

func (entity *Entity) Init() {}

func (entity *Entity) Update() {}

func (entity *Entity) Destroy() {}

func (entity *Entity) OnMsg(source uint64, msg Msg) {}

func RegisterEntity(base IBase) {
	entity := GetEntity(base)
	if entity == nil {
		return
	}
	entityID := entity.GetID()
	_, ok := EntityMap[entityID]
	if ok == false {
		EntityMap[entityID] = base
	}
}

func GetEntity(owner IBase) IProperty {
	if entity, ok := owner.(IProperty); ok {
		return entity
	}
	return nil
}

func GetEntityByID(id uint64) IBase {
	if entity, ok := EntityMap[id]; ok {
		return entity
	}
	return nil
}

func GetProperty(id uint64) IProperty {
	base := GetEntityByID(id)
	property := GetEntity(base)
	return property
}

func GetAvatar(base IBase) *Avatar {
	if entity, ok := base.(*Avatar); ok {
		return entity
	}
	return nil
}

func GetEnemy(base IBase) *Enemy {
	if entity, ok := base.(*Enemy); ok {
		return entity
	}
	return nil
}

func CreateEntity() Entity {
	entity := Entity{
		id:        globalID,
		position:  defaultPosition,
		rotation:  defaultRotation,
		hp:        defaultHP,
		mp:        defaultMP,
		moveSpeed: defaultMoveSpeed,
	}
	entity.Init()
	globalID = globalID + 1
	return entity
}
