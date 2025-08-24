## system
RT-Thread v5.2.0 Released
## chip
STM32L4 Pandora

## lsp config
### clangd
1. `{workspace}/.clangd`
2. `{workspace}/.clang-format`
3. `pandora/.clangd`
4. `pandora/.clang-format`
5.  `.vscode`

## compile
### RT-thread env 
```bash
scons
pkgs --update
pkgs --upgrade
```
### keil
```bash
scons --target=mdk5
```
## peripherials
1. i2c
2. sdio

