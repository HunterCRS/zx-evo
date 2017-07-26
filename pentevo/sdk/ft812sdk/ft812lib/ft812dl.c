
{
  return (9UL << 24) | ((func & 7L) << 8) | ((ref & 255L) << 0);
}

{
  return (31UL << 24) | prim;
}

{
  return (5UL << 24) | handle;
}

{
  return (7UL << 24) | ((format & 31L) << 19) | ((linestride & 1023L) << 9) | ((height & 511L) << 0);
}

{
  return (0x28UL << 24) | ((linestride & 3L) << 2) | ((height & 3L) << 0);
}

{
  return (0x28UL << 24) | (((linestride >> 10) & 3L) << 2) | (((height >> 9) & 3L) << 0);
}

{
  u8 fxy = (filter << 2) | (wrapx << 1) | (wrapy);
  return (8UL << 24) | ((u32)fxy << 18) | ((width & 511L) << 9) | ((height & 511L) << 0);
}

{
  return (0x29UL << 24) | ((width & 3L) << 2) | ((height & 3L) << 0);
}

{
  return (0x29UL << 24) | (((width >> 9) & 3L) << 2) | (((height >> 9) & 3L) << 0);
}

{
  return (1UL << 24) | ((addr & 0x3FFFFFL) << 0);
}

{
  return (21UL << 24) | ((a & 131071L) << 0);
}

{
  return (22UL << 24) | ((b & 131071L) << 0);
}

{
  return (23UL << 24) | ((c & 16777215L) << 0);
}

{
  return (24UL << 24) | ((d & 131071L) << 0);
}

{
  return (25UL << 24) | ((e & 131071L) << 0);
}

{
  return (26UL << 24) | ((f & 16777215L) << 0);
}

{
  return (11UL << 24) | ((src & 7L) << 3) | ((dst & 7L) << 0);
}

{
  return (29UL << 24) | ((dest & 2047L) << 0);
}

{
  return (6UL << 24) | ((cell & 127L) << 0);
}

{
  return (15UL << 24) | ((alpha & 255L) << 0);
}

{
  return (2UL << 24) | ((red & 255L) << 16) | ((green & 255L) << 8) | ((blue & 255L) << 0);
}

{
  return (2UL << 24) | (rgb & 0xffffffL);
}

{
  u8 m = (c << 2) | (s << 1) | t;
  return (38UL << 24) | m;
}

{
  return (38UL << 24) | 7;
}

{
  return (17UL << 24) | ((s & 255L) << 0);
}

{
  return (18UL << 24) | ((s & 255L) << 0);
}

{
  return (16UL << 24) | ((alpha & 255L) << 0);
}

{
  return (32UL << 24) | ((r & 1L) << 3) | ((g & 1L) << 2) | ((b & 1L) << 1) | ((a & 1L) << 0);
}

{
  return (4UL << 24) | ((red & 255L) << 16) | ((green & 255L) << 8) | ((blue & 255L) << 0);
}

{
  return (4UL << 24) | (rgb & 0xffffffL);
}

{
  return 0UL << 24;
}

{
  return 33UL << 24;
}

{
  return (30UL << 24) | ((dest & 2047L) << 0);
}

{
  return (14UL << 24) | ((width & 4095L) << 0);
}

{
  return (37UL << 24) | ((m & 1L) << 0);
}

{
  return (13UL << 24) | ((size & 8191L) << 0);
}

{
  return 35UL << 24;
}

{
  return 36UL << 24;
}

{
  return 34UL << 24;
}

{
  return (28UL << 24) | ((width & 1023L) << 10) | ((height & 1023L) << 0);
}

{
  return (27UL << 24) | ((x & 511L) << 9) | ((y & 511L) << 0);
}

{
  return (10UL << 24) | ((func & 7L) << 16) | ((ref & 255L) << 8) | ((mask & 255L) << 0);
}

{
  return (19UL << 24) | ((mask & 255L) << 0);
}

{
  return (12UL << 24) | ((sfail & 7L) << 3) | ((spass & 7L) << 0);
}

{
  return (20UL << 24) | ((mask & 1L) << 0);
}

{
  return (3UL << 24) | s;

u32 ft_Vertex2f(s16 x, s16 y)
  return (1UL << 30) | ((x & 32767L) << 15) | ((y & 32767L) << 0);
}

u32 ft_Vertex2ii(u16 x, u16 y, u8 handle, u8 cell)
}

u32 ft_VertexFormat(u8 f)
  return (0x27UL << 24) | (f & 7L);