bits 16
add bx, [bx + si]
add bx, [bp]
add word , 2
add word , 2
add word , 8
add bx, [bp]
add cx, [bx + 2]
add bh, [bp + si + 4]
add di, [bp + di + 6]
add [bx + si], bx
add [bp], bx
add [bp], bx
add [bx + 2], cx
add [bp + si + 4], bh
add [bp + di + 6], di
add byte [bx], 34
add word [bp + si + 1000], 29
add ax, [bp]
add al, [bx + si]
add ax, , bx
add al, , ah
add (null), 1000
add (null), 226
add (null), 9
sub bx, [bx + si]
sub bx, [bp]
sub word , 2
sub word , 2
sub word , 8
sub bx, [bp]
sub cx, [bx + 2]
sub bh, [bp + si + 4]
sub di, [bp + di + 6]
sub [bx + si], bx
sub [bp], bx
sub [bp], bx
sub [bx + 2], cx
sub [bp + si + 4], bh
sub [bp + di + 6], di
sub byte [bx], 34
sub word [bx + di], 29
sub ax, [bp]
sub al, [bx + si]
sub ax, , bx
sub al, , ah
sub (null), 1000
sub (null), 226
sub (null), 9
cmp bx, [bx + si]
cmp bx, [bp]
cmp word , 2
cmp word , 2
cmp word , 8
cmp bx, [bp]
cmp cx, [bx + 2]
cmp bh, [bp + si + 4]
cmp di, [bp + di + 6]
cmp [bx + si], bx
cmp [bp], bx
cmp [bp], bx
cmp [bx + 2], cx
cmp [bp + si + 4], bh
cmp [bp + di + 6], di
cmp byte [bx], 34
cmp word [4834], 29
cmp ax, [bp]
cmp al, [bx + si]
cmp ax, , bx
cmp al, , ah
cmp (null), 1000
cmp (null), 226
cmp (null), 9
