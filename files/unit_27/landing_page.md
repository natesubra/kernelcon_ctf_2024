[Unit 27](https://www.youtube.com/watch?v=XvUgI4c5zZc

The server was xz backdoored using a key generated with:

package main
import (
	"fmt"
	"math/big"
	"math/rand"
	"time"
	"github.com/cloudflare/circl/sign/ed448"
)
func main() {
	rand.Seed(time.Now().UnixNano())
	randomNumber := rand.Intn(10000000)
	var seed [ed448.SeedSize]byte
	sb, _ := new(big.Int).SetString(fmt.Sprintf("%d", randomNumber), 10)
	sb.FillBytes(seed[:])
	key := ed448.NewKeyFromSeed(seed[:])
	fmt.Printf("%x", key[ed448.SeedSize:])
}

The key started with 6e6c9a9bb7

If the challenge is broken contact the organizers to fix.
