---
layout:     post
title:      "Mathematics for Machine Learning 笔记"
subtitle:   "机器学习数学基础  —— 学习笔记"
date:       2018-10-20 19:00:00
author:     "Becks"
header-img: "img/post-bg2.jpg"
catalog:    true
published: false
tags:
    - Coursera
    - Machine Learning
    - 学习笔记
---
> note from Coursera Mathematics for Machine Learning
> 

## Course 1: Linear Algebra

<script type="text/javascript" async src="https://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-MML-AM_CHTML"> </script>

#### vector

<span style="background-color: #FFFF00">property: </span>

Communicative: &nbsp;&nbsp;&nbsp;  $$ \vec v \cdot \vec w  =  \vec w \cdot \vec v $$ <br/>
Distributive: &nbsp;&nbsp;&nbsp;  $$ \left(\vec w + \vec v \right)\cdot \vec x  =  \vec w \cdot \vec x + \vec v \cdot \vec x  $$<br/>
Associative over scaler multiplication: &nbsp;&nbsp;&nbsp; $$ \left( c \vec v \right) \cdot \vec w  =  c \left( \vec v \cdot \vec w \right)  $$<br/>
Dot product self is the length square: &nbsp;&nbsp;&nbsp; $$ c\vec v \cdot \vec v  =  ||v||^2 = v_1^2 + v_2^2 + ... + v_n^2  $$<br/>
Cosine: &nbsp;&nbsp;&nbsp; $$  \vec a \cdot \vec b  =  ||a||^2||b||^2 cos\theta  $$<br/>
scaler projection: &nbsp;&nbsp;&nbsp; $$  \vec proj_{L} \left(\vec x\right) =   \frac{ \vec x \cdot \vec v  }{ ||\vec v|| } $$<br/>
vector projection: &nbsp;&nbsp;&nbsp; $$  \vec proj_{L} \left(\vec x\right) =  c \vec v =  \frac{ \vec x \cdot \vec v  }{ \vec v \cdot \vec v } \vec v $$<br/>
two vector $$  \vec  x $$ and $$  \vec y $$ are orthorgonal: &nbsp;&nbsp;&nbsp; $$  \vec  x \cdot \vec y  = 0 $$ 


注: <span style="color: red">difference between perpendicular and orthogonal</span>:  $$  \vec  x \cdot \vec y  = \vec 0 $$   only means orthogonal, zero vector is orthogonal to everything but zero vector not perpendicular to everything; perpendicular is orthogonal, 但是othogonal 不一定是perpendicular, 因为 $$  \vec  x \cdot \vec 0  = 0 $$ 不是perpendicular

求basis的coordinate的时，若basis vector orthogonal to each other, 可以用scaler projection, 看$$  \vec x$$到$$  \vec v_i$$的projection $$ \frac{ \vec x \cdot \vec v  }{ \|\vec v\|^2 } $$ 即是coordinate

#### basis

Basis is a set of n vectors that 
- are not linear combinations of each other (linear independent): <br/>
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; for all $$ a_1, …, a_n \subseteq F $$, if $$ a_1v_1 + … + a_nv_n = 0 $$, then necessarily  $$ a_1 = … = a_n = 0;  $$
- span the space: <br/> 
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; every (vector) $$\vec x$$ in V it is possible to choose $$ a_1, …, a_n \subseteq  F $$ such that $$ x = a_1 \vec v_1 + … + a_n \vec vn. $$
- The space is then n-dimensional 

```python
import numpy as np
#calculate inverse
A = [[1, 1, 3],
     [1, 2, 4],
     [1, 1, 2]]
Ainv = np.linalg.inv(A)

#solve linear system
A = [[4, 6, 2],
     [3, 4, 1],
     [2, 8, 13]]

s = [9, 7, 2]

r = np.linalg.solve(A, s)

#calculate 长度norm
B = np.array(A, dtype=np.float_)
la.norm(B[:, 1])

#Gram-Schmidt process
def gsBasis(A) :
    B = np.array(A, dtype=np.float_) # Make B as a copy of A, since we're going to alter it's values.
    # Loop over all vectors, starting with zero, label them with i
    for i in range(B.shape[1]) :
        # Inside that loop, loop over all previous vectors, j, to subtract.
        for j in range(i) :
            # Complete the code to subtract the overlap with previous vectors.
            # you'll need the current vector B[:, i] and a previous vector B[:, j]
            B[:, i] = B[:,i] - B[:, i]@B[:,j]*B[:,j]
        # Next insert code to do the normalisation test for B[:, i]
        if la.norm(B[:,i]) > verySmallNumber:
            B[:, i] = B[:, i] / la.norm(B[:, i])
        else :
            B[:, i] = np.zeros_like(B[:, i])     
            
    # Finally, we return the result:
    return B

#dot product @
B[:, i]@B[:,j]


#矩阵乘法 @
A@B 
"""
[1, 2]   [5,  6]  =    [19, 22]
[3, 4]   [7,  8]  =    [43, 50]

A*B 是elementwise 乘法
[1, 2]   [5,  6]  =    [5,  12]
[3, 4]   [7,  8]  =    [21, 32]

"""

#calculate eigenvector and eigenvalue

M = np.array([[1, 0, 0],
              [0, 2, 0],
              [0, 0, 3]])
vals, vecs = np.linalg.eig(M)
vals
```