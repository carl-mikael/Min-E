Learning about DOD, data oriented design.
https://www.youtube.com/watch?v=IroPQ150F6c

- Structure of arrays increases cache locality when performing operations on a specific attribute/data. It also removes padding.
- If possible use a 4byte int, instead of pointers as they take 8bytes.
- Avoid storing the type bool. It has the cost of at least 1 byte, often more thanks to padding, and only stores the information worth of 1 bit.
- Instead of storing a bool, you could remove the field and store the struct in two different arrays instead.
- If the struct contains information that's rarely used, i.e. an NPCs health, it could be stored separately and be looked up only when needed. If we want to update all NPCs positions every frame, it would be unnecessary to waste cache space on their health.
- Math instruction are extremely cheap on the CPU, memory instructions are not. Consider lazily calculating values over saving/looking them up. (Lazily calculating something is calculating it on the spot when you need it).
- You could interpret data in different ways depending on context. The guy in the video talk called it encoding. Can sometimes be done over polymorphism.

This week we have the choice of creating an ECS or do text rendering. I'm not very interested in text rendering so I'll do an "ESC". Though, the more I think about ECS, the less I like they way they want us to do it in the assignment.
If my understanding of ESC is correct, the the goal with it is to add cache locality, usually at the price of complexity. But, if you have entity can have an empty component that takes up space, that works against that. And, what's the point of locality of data if you don't use procedures on the local data? I.e. why would I have a component type which holds data that I don't run procedures on? Like, what's the point of that. Wouldn't it be better to just not stored that thing as a component, i.e. on the entity directly or somewhere else indirectly if it isn't used often with the entity.