package main

import (
	PP2PLink "SD/PP2PLink"
	"bufio"
	"fmt"
	"os"
)

func main() {

	if len(os.Args) < 2 {
		fmt.Println("Usage:   go run pp2plTest.go thisProcessIpAddress:port otherProcessIpAddress:port")
		fmt.Println("Example: go run pp2plTest.go  127.0.0.1:8050    127.0.0.1:8051")
		fmt.Println("Example: go run pp2plTest.go  127.0.0.1:8051    127.0.0.1:8050")
		return
	}

	addresses := os.Args[1:]
	fmt.Println("Chat PPLink - addresses: ", addresses)

	lk := PP2PLink.NewPP2PLink(addresses[0], false)

	go func() {
		for {
			m := <-lk.Ind
			fmt.Println("                                            Rcv: ", m)
		}
	}()

	go func() {
		for {
			fmt.Print("Snd: ")
			scanner := bufio.NewScanner(os.Stdin)
			var msg string

			if scanner.Scan() {
				msg = scanner.Text()
			}
			req := PP2PLink.PP2PLink_Req_Message{
				To:      addresses[1],
				Message: msg}

			// for i := 1; i < 100; i++ {
			lk.Req <- req
			//}
		}
	}()

	<-(make(chan int))
}
