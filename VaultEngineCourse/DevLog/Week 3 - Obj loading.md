https://www.youtube.com/watch?v=TH9VCN6UkyQ&t=4042s

Learning about memory.
- I've had memory alignment on my mind for a few days and it finally clicked. I was so confused about how it solved anything/why it was needed. I've finally realized that a types
- CPU has load sizes: 1, 2, 4, 8, 16, 32, and 64.
- The cache line is 64 bytes and moves mem between ram and cache
- A page is by default 4kb. The OS distributes ram as pages to applications.
- Alignment ensures mem fits to its load size, and therefore can be read with a single instruction. In cpp, trying to access mem at an address that isn't a multiple of its alignment is undefined behavior. (This last line still messes with my head)
- Registers -> Cache L1->l2->l3 -> RAM -> SSD/HDD
- A load is mem -> register
- You need SIMD, single instruction multiple data, to use load size of 32+ using x64.
	- My understanding is that load size of 64 isn't very supported so even with SIMD the go to size is 32 bytes.
	- You need to specify arguments when compiling
	- mm_loadu_ps(address) to load the address using SIMD
	- An x64 CPU word is it's normal register size, 8bytes (64 bits).
		- It has special SIMD registers holding 16bytes, but they are hardware separated and therefore aren't used for the general purpose tasks of storing integers, addresses, and loop counters. Software can work around this but at a cost. I.e. float and double math use SIMD registers.
	- SIMD has its own registers with size 16, 32, and 64.
		- The hardware allows for instructions that manipulate the data in the registers using "slots". Meaning an instruction could handle 2 registers data as if they are holding multiple values, in order to perform the same operation on multiple values at once. I.e. 2 SIMD 16 bytes registers could hold 4 int:s each and with a single CPU instruction do 4 additions at once. Register 1 first slot + register 2 first slot and so on.
		- The instruction defines how many "slots" to interpret a register as, 2, 4, 8, 16, for a 16byte register, which would leave each slot with 8, 4, 2, or 1 bytes. It can be used as 1 slot/a chunk of 16 bytes, but only for specific instructions like load, and bitwise operations.