

<pre lang="txt">
<code>
Nachos File System
+-----------------------------------+
|             File User             |
+-----------------------------------+
| FileSystem | OpenFile | Directory |
+------------+----------+-----------+
|            File Header            |
+-----------------------------------+
|             SynchDisk             |
+-----------------------------------+
|                Disk               |
+-----------------------------------+
</code>
</pre>

## filesys.cc
用文件名代表文件。
单个文件的操作定义在<code>OpenFile.h</code>中。
   
- 文件系统中的**每个文件**都具有：
  - 文件头
存储在磁盘的扇区中
文件头数据结构的大小正好安排为**1个磁盘扇区**的大小
  - 多个数据块
   - 文件系统目录中的项


- 文件系统两个关键的数据结构（本身作为文件存储在文件系统中）：
  - **“根”目录**directory 
文件名和文件头的目录。**只有一个**，列出文件系统所有文件，不提供分层目录结构
  - **位图**bitmap
可用磁盘扇区的位图，用于分配磁盘扇区

**位图**和**目录**都表示为**普通文件**。它们的文件头位于特定的扇区（扇区0和扇区1）中，以便文件系统在启动时可以找到它们。


- 文件系统假定Nachos运行时，位图和目录文件连续保持“**打开**”状态。

对于**修改**目录和/或位图的那些操作（例如Create，Remove）
- 如果操作成功
则所做的更改将立即写回到磁盘上（这两个文件一直保持打开状态）。
- 如果操作失败，并且我们已经修改了目录和/或位图的一部分
则只需丢弃更改的版本，而无需将其写回到磁盘。

实现具有以下限制：

- 并发访问没有同步
- 文件的大小是固定的，创建文件时设置的大小不能大于3KB
- 没有分层目录结构
- 只能向系统添加数量有限的文件
- 没有尝试使系统对故障具有鲁棒性（如果Nachos在修改文件系统的操作中间退出，则可能会损坏磁盘）

### 成员变量

- OpenFile* freeMapFile;		
可用磁盘扇区的位图
- OpenFile* directoryFile;		
根目录，文件名和文件头的目录
};


### 操作
#### FileSystem(bool format);	
初始化文件系统。 
必须在“ synchDisk”已初始化之后。
如果为“ format”，则磁盘上没有任何内容，需要初始化空目录和可用块的位图。否则打开位图和目录。

#### bool Create(char *name, int initialSize);  	
 创建一个文件 (UNIX creat)
 Nachos不允许动态改变文件大小，所以初始化就要确定文件大小。
 1. 确认文件不存在
 2. 为文件头分配扇区
 3. 为文件数据块分配磁盘空间
 4. 向目录添加文件名
 5. 把bitmap和directory写回
#### OpenFile* Open(char *name);
打开一个文件  (UNIX open)

#### bool Remove(char *name);
删除一个文件 (UNIX unlink)
包括：
- 从目录中移除
- 删除文件头
- 删除数据块
- 将directory bitmap写回

#### void List();
列出文件系统上所有文件

#### void Print();	
列出所有文件及内容

### 定义
- 位图、目录
**位图**和**目录**都表示为**普通文件**。它们的文件头位于特定的扇区（扇区0和扇区1）中，以便文件系统在启动时可以找到它们。

```c
#define FreeMapSector 		0
#define DirectorySector 	1
```

<pre lang="txt">
<code>
Disk Allocation Structure

+----+----+---------------------+
| 0# | 1# | Normal Storage Area |
+----+----+---------------------+
  |     |
  |    1#: Root directory's i-node
  |
 0#: System bitmap file's i-node
</code>
</pre>
- 初始大小
在文件系统支持可扩展文件之前，目录大小将设置可以加载到磁盘上的最大文件数。

**位图**的初始大小：
（以byte为单位）
```c
#define FreeMapFileSize 	(NumSectors / BitsInByte)
```
磁盘中总扇区数<code>NumSectors</code>定义在<code>machine/disk.h</code>
```c
//...
#define SectorsPerTrack 	32	// number of sectors per disk track 
#define NumTracks 		32	// number of tracks per disk
#define NumSectors 		(SectorsPerTrack * NumTracks)
					// total # of sectors per disk
//...
```
<code>BitsInByte</code>定义在<code>userprog/bitmap.h</code>
```c
//...
// 用整数数组代表bitmap
#define BitsInByte 	8
//...
```

**目录**的初始大小：
目录项数（最多允许创建10个目录）* 每项的大小
```c
#define NumDirEntries 		10
#define DirectoryFileSize 	(sizeof(DirectoryEntry) * NumDirEntries)
```



### 函数详解
#### FileSystem(bool format);	
- 若format == TRUE
需要格式化，初始化磁盘信息。主要把位图和目录装载到磁盘中去。如前文所述，位图和目录都以普通文件形式存在，而一个文件都具有文件头和目录项。
  1. 新建  freeMap [BitMap *]、directory [Directory *]，新建文件头mapHdr、dirHdr[FileHeader *]
  ```c
  BitMap *freeMap = new BitMap(NumSectors);
  Directory *directory = new Directory(NumDirEntries);
	FileHeader *mapHdr = new FileHeader;
	FileHeader *dirHdr = new FileHeader;

  ```
  2. 将0、1号扇区分配给位图和目录的文件头
  ```c
	freeMap->Mark(FreeMapSector);	    
	freeMap->Mark(DirectorySector);
  ```
  3. 为包含目录和位图文件内容的数据块分配空间（初始大小）
  这一步将修改freeMap
  ```c
  ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
	ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));

  ```
  4. 将文件头mapHdr、dirHdr写回磁盘
  因为下一步OpenFile操作要读取磁盘相应扇区
  ```c
  mapHdr->WriteBack(FreeMapSector);    
	dirHdr->WriteBack(DirectorySector);
  ```
  5. 打开文件freeMapFile和directoryFile[OpenFile *]
  读0、1扇区
  这两个文件在Nachos运行期间保持打开
  ```c
  
        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
  ```
  6. 写回freeMap 、directory

  ```c
  	freeMap->WriteBack(freeMapFile);	 // flush changes to disk
	  directory->WriteBack(directoryFile);
  ```
- 若format == FALSE
不需要初始化磁盘
直接打开两个文件即可


## filehdr.cc
用于管理磁盘文件头的数据结构。
- 文件头用于定位文件数据在磁盘上的存储位置，以及有关文件的其他信息（例如，文件的长度、所有者等）
（在UNIX术语中，是“iNode”）。
- 与实际系统不同，我们不跟踪文件头中的文件权限、所有权、上次修改日期等。
- 我们将其实现为一个固定大小的指针表——表中的每个条目都指向包含该部分文件数据的磁盘扇区（换句话说，没有间接或双间接块）。


- 文件头数据结构可以存储在内存或磁盘上。
当它在磁盘上时，它存储在一个扇区中——这意味着我们假设这个数据结构的大小与一个磁盘扇区的大小相同。不受最大文件长度限制的间接寻址。

- 没有构造函数，文件头可以用两种方式初始化：
  - 【Allocate】对于新文件，通过修改内存中的数据结构以指向新分配的数据块
  - 【FetchFrom】对于已经在磁盘上的文件，通过从磁盘读取文件头

### 成员
- 文件的字节数 numByte
- 文件的数据扇区数 numSectors;		
- 每个数据块的扇区号 dataSectors[NumDirect];	
因为文件头正好占一个扇区，所以能除去前两个保存的int类型的信息，还能存NumDirect个数据块。


## directory.cc
目录是一个由对组成的表：<文件名，扇区#>，给出目录中每个文件的名称，以及在磁盘上找到其文件头（描述在何处找到文件数据块的数据结构）的位置。

- 规定文件名不超过9个字符

### DirectoryEntry
目录项，在目录中代表一个文件，包含文件名和文件头在磁盘上的位置。
类型|变量名|描述
-|-|-
bool|inUse|目录项是否被使用
int|sector|此文件的文件头在磁盘上的位置
char数组|name|文件名

### Directory
目录，可以存储在内存或者磁盘上，磁盘上以Nachos文件形式。
构造函数在内存初始化目录结构，FetchFrom和WriteBack操作从磁盘读写目录信息。
#### 成员变量
类型|变量名|描述
-|-|-
int|tableSize|目录项数
DirectoryEntry *|table|目录项们

## openfile.cc
打开、关闭、读写单个文件
### 成员变量
- FileHeader *hdr
文件头
- int seekPsition
当前位置

### 操作

#### Read && Write
从seekPosition开始读写，返回实际读写的字节数。通过调用ReadAt和WriteAt，但是会将seekPsition改变为读写后的位置。

#### ReadAt && WriteAt
从seekPosition开始读/写文件的一部分。
返回实际写入或读取的字节数，但不修改seekPosition。
不能保证请求在均匀的磁盘扇区边界上开始或结束；但是磁盘一次只知道如何读/写整个磁盘扇区。因此：
- ReadAt
读取请求中的所有完整或部分扇区，但只复制感兴趣的部分。
- WriteAt
我们必须首先读取将部分写入的任何扇区，这样就不会覆盖未修改的部分。然后，我们复制要修改的数据，并写回请求中的所有完整或部分扇区。
