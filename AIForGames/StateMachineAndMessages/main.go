// StateMachineAndMessages project main.go
package main

import (
	"fmt"
)

func onceUpdate(entities []IBase) {
	for _, entity := range entities {
		entity.Update()
	}
}
func main() {
	fmt.Println("Hello World!")
	avatar := CreateAvatar()
	enemy := CreateEnemy()
	entities := []IBase{avatar, enemy}
	onceUpdate(entities)
	Dispatch(avatar.GetID(), enemy.GetID(), MSG_ATTACK, avatar)
	avatar.SetCurrentState(
		&AvatarAttackState{
			source:      avatar.GetID(),
			destination: enemy.GetID(),
		})
	onceUpdate(entities)
	avatar.ShiftToPreviousState()
	// for i := 0; i < 5; i++ {
	// 	for _, entity := range EntityMap {
	// 		entity.Update()
	// 	}
	// }
}
