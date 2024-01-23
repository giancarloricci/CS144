Checkpoint 1 Writeup
====================

My name: Giancarlo Ricci

My SUNet ID: gricci

I collaborated with: laya, aguman

I would like to thank/reward these classmates for their help: laya, aguman

This lab took me about 12 hours to do. I did attend the lab session.

I was surprised by or edified to learn that: it was really cool to see the in class discussion of the reassembler and how it's used by TCP to turn unreliable UDP datagrams into reliable programs. I think that my initial difficulty in appreciating why capacity needed to be enforced were clarified by thinking about the constraints that TCP would adhere to. I would've loved in Monday (1/22) lecture would've been sooner, as I felt that it really motivated and helped me appreciate the work we were doing in this lab. 

Describe Reassembler structure and design. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

The fundamental approach I used is to use a container which stores:
a. The first index of the string
b. The literal string 

This approach is useful, because there are two problems that need to be solved: 

a. The ability to add new data into storage
b. The ability to remove data in storage that is ready to be pushed.

This data structure allows each of these problems to be solved, and prevents issues including:
adding redundant information to storage, not removing all possible data from storage in one pass.

The actual container is subject to debate. I originally wanted to use a map; however, after discussing with TA Isaac during lab, I realized a vector would probably be more efficient. Furthermore, during my implemtation of storage_insert(), I realized that specific inserts into memory might be made more efficient using a list instead of a vector through the emplace methods after reading the following posts:

a. https://stackoverflow.com/questions/2209224/vector-vs-list-in-stl
b. https://stackoverflow.com/questions/14788261/c-stdvector-emplace-vs-insert

While this approach had sufficient information to implement these helper functions, I wonder whether a simpler datastructure might have been able to streamline some of the case handling.

Implementation Challenges: The biggest challenges I dealt with were the two helper functions I had to write. In particular, I had to do a lot of specific case handling to ensure redundant information was not added. I've seen other ideas floating around on Ed, such as using two strings. I can imagine that different approaches might have simplified the logic to write these two helper functions.

Remaining Bugs: There are no bugs that I'm currently aware of.

- Optional: I had unexpected difficulty with: Edge case handling

- Optional: I think you could make this lab better by: Maybe providing more start code / direction in the handout. I'm not sure if it was just my datastructure, but the code was more hairy than I was expecting (and significantly more work then Lab0).

- Optional: I'm not sure about: N/A
