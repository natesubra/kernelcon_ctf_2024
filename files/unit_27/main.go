package main

import (
    "fmt"
    "math/big"
    "github.com/cloudflare/circl/sign/ed448"
    "strings"
    "time"
)

func main() {
    startTime := time.Now() // Track when we started
    printInterval := 2 * time.Second // How often to print the current number

    for randomNumber := 0; randomNumber < 10000000; randomNumber++ {
        // Check if it's time to print the current number
        if time.Since(startTime) >= printInterval {
            fmt.Printf("Currently testing number: %d\n", randomNumber)
            startTime = time.Now() // Reset the start time
        }

        // Convert randomNumber to seed
        var seed [ed448.SeedSize]byte
        sb, _ := new(big.Int).SetString(fmt.Sprintf("%d", randomNumber), 10)
        sb.FillBytes(seed[:])

        // Generate the key from the seed
        key := ed448.NewKeyFromSeed(seed[:])

        // Convert the part of the key after the seed size to hex string
        keyHex := fmt.Sprintf("%x", key[ed448.SeedSize:])

        // Check if the generated key begins with the known prefix
        if strings.HasPrefix(keyHex, "6e6c9a9bb7") {
            fmt.Printf("Match found with randomNumber %d: %s\n", randomNumber, keyHex)
            break // Stop the search once a match is found
        }
    }
}
