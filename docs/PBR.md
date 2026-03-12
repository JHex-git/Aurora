# PBR

This document summarizes the physically based rendering (PBR) shading model used in Aurora.

**BRDF**
- The BRDF splits into diffuse and specular. Diffuse uses Lambert. The specular component follows a microfacet model: the surface is treated as many tiny mirrors whose orientation distribution, visibility, and Fresnel response are described by $D$, $G$, and $F$.
- The outgoing radiance is the integral of diffuse plus specular:
$$
\begin{aligned}
L_o &= \int_{\Omega} (f_{diffuse} + f_{specular}) \cdot L_i(\omega_i) \cdot (N \cdot \omega_i)\, d\omega_i
\end{aligned}
$$
Here $L_i(\omega_i)$ is incoming radiance from direction $\omega_i$. In practice the integral is evaluated as a sum over lights and visibility.

- Energy conservation and the diffuse/specular split:
$$
\begin{aligned}
k_S &= F \\
k_D &= (1 - k_S)(1 - metallic) \\
f_{diffuse} &= \frac{k_D \cdot albedo}{\pi} \\
f_{specular} &= \frac{D \cdot G \cdot F}{4 (N \cdot V)(N \cdot L)}
\end{aligned}
$$

- The specular term uses Cook-Torrance with GGX $D$, Schlick-GGX geometry, and Schlick Fresnel:
$$
\begin{aligned}
D &= \frac{a^2}{\pi \left((N \cdot H)^2 (a^2 - 1) + 1\right)^2} \\
G &= G_{Schlick}(N \cdot V) \cdot G_{Schlick}(N \cdot L) \\
G_{Schlick}(N \cdot V) &= \frac{N \cdot V}{(N \cdot V)(1-k) + k}, \quad k = \frac{(r+1)^2}{8} \\
F &= F_0 + (1 - F_0)(1 - (H \cdot V))^5
\end{aligned}
$$
Here $D$ reflects how many microfacet normals align with the half vector, $G$ reflects masking on the outgoing (view) direction and shadowing on the incoming (light) direction, and $F$ reflects the Fresnel fraction of incident light that is reflected.
Here $a=r^2$, $r$ is roughness, $F_0$ is base reflectance, $H$ is the half vector, and $N$, $V$, $L$ are normal, view, and light directions.

**Ambient and Emissive**
- Ambient is applied as a small constant term modulated by AO:
$$
\begin{aligned}
ambient = k_{ambient} \cdot albedo \cdot ao
\end{aligned}
$$
- Aurora uses $k_{ambient} = 0.03$.
- Emissive is added after lighting as an unshadowed contribution.
