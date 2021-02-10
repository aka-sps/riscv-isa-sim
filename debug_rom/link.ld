OUTPUT_ARCH( "riscv" )
ENTRY( entry )
SECTIONS
{
    .whereto 0x1300 :
    {
        *(.whereto)
    }   
    . = 0x1800;
    .text :
    {
        *(.text)
    }
    _end = .;
}
