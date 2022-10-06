package main

type MsgType uint8

const (
	MSG_ATTACK MsgType = 1
)

type Msg struct {
	mType       MsgType
	source      uint64
	destination uint64
	extra       interface{}
}

func Dispatch(source uint64, destination uint64, msgType MsgType, extra interface{}) {
	sourceEntity := GetEntityByID(source)
	if sourceEntity == nil {
		return
	}
	destinationEntity := GetEntityByID(destination)
	if destinationEntity == nil {
		return
	}
	msg := Msg{
		mType:       msgType,
		source:      source,
		destination: destination,
		extra:       extra,
	}
	destinationEntity.OnMsg(source, msg)
}
