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


## hardware

### spi 
STM32L4
1. IO_PC6  SPI_CS
2. IO_PB13 SPI2_SCK
3. IO_PB14 SPI2_MISO
4. IO_PB15 SPI2_MOSI

orangepi
1. DIP26-3 SPI1_CS0
2. DIP26-24 SPI1_CS1
3. DIP26-23 SPI1_CLK
4. DIP26-21 SPI1_MISO
5. DIP26-19 SPI1_MOSI

