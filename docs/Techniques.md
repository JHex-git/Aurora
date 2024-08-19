Here list all techniques implemented in this simple render engine, some implementation details are introduced.

# Outline

TODO

# Shadow

## Shadow Mapping

对每个光源，从光源出发向PosX、NegX、PosY、NegY、PosZ、NegZ渲染场景深度，生成cubemap作为shadow map，这里需要注意opengl中cubemap的渲染方向（Opengl中垂直方向是颠倒的）。

![image-20240817141044522](images/cubemap.png)

在下一个pass中，采样场景中的点到目标光源的距离，与光源到该方向最近的遮挡物的距离进行对比。由于shadow map中存放的$z$是非线性深度，需要将它恢复成线性深度，并据此计算距离。

由透视投影公式

$$
zClip = \frac{far + near}{far - near} + \frac{1}{depth} * (\frac{-2\cdot far \cdot near}{far - near})
$$

可以推得

$$
depth = \frac{far\cdot near}{far -z\cdot(far-near)}
$$

此外还需解决数据精度以及shadow map是离散的导致的自遮挡问题，引入了光照和法线的夹角作为 $\epsilon$ 的系数，并限定了 $\epsilon$ 最小值。

![image-20240817142237975](images/self_shadow.png)
