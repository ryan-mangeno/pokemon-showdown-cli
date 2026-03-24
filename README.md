# pokemon-showdown-app

Multiplayer pokemon battle client with a native macOS UI and low-level C++ networking, I will likely end up integrating this with my Acorn UI framework with C++ and Vulkan.

Note - this is just getting started, and I am currently going to shoot for a simple MVP, being able to enter a random 6v6 battle, then I will add more features.

Connects to [Pokémon Showdown](https://pokemonshowdown.com) — the server is open source so I can determine API endpoints and protocol compatibility. This repo may break if upstream changes their protocol, and I am the only maintainer.

- Server reference: https://github.com/smogon/pokemon-showdown
- Protocol docs: https://github.com/smogon/pokemon-showdown/blob/master/PROTOCOL.md

## Dependencies

```bash
brew install boost openssl
```

## Build

```bash
mkdir build
cmake -B build -DOPENSSL_ROOT_DIR=$(brew --prefix openssl)
make
```

## Run

```bash
./build/ps-cli
```

## CLI 
<img width="1338" height="1016" alt="Screenshot 2026-03-24 at 4 05 45 PM" src="https://github.com/user-attachments/assets/188611c6-48ef-43e3-ba7a-c006e9e007d7" />
