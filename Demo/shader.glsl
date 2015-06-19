uint seed;

float rand()
{
    seed ^= seed << 1;
    seed ^= seed >> 3;
    seed ^= seed << 10;
    return float(seed % 10000) / 10000.0;
}

// distance function to the Menger Sponge
float menger(vec3 pos)
{
    vec3 pos2 = abs(pos *= 0.5) - 0.5;
    pos += 0.5;
    float d = max(max(pos2.x, pos2.y), pos2.z);
    float p = 1.0;
    for (int i = 1; i < 8; i++)
    {
        p *= 3.0;
        pos2 = 0.5 - abs(mod(pos*p, 3.0) - 1.5);
        d = max(min(max(pos2.x, pos2.z),
                min(max(pos2.x, pos2.y), 
                    max(pos2.y, pos2.z))) / p, d);
    }
    return d*2.0;
}

void main()
{
    vec3 color = vec3(0.0);
    vec2 eps = vec2(0.0001, 0.0);
    vec2 coord = gl_FragCoord.xy;
    seed = 1000 * uint(coord.x) + uint(coord.y);

    for (int it = 0; it < 100; it++)
    {
        vec3 weight = vec3(2.0); // recursive color depth
        vec3 ro = vec3(0.0, 0.0, -1.0); 

        // ray direction
        vec3 rd = vec3((coord + 1.5 * vec2(rand(), rand())) / 1080.0 - vec2(1920.0/1080.0/2, 0.5), 1.0);

        // Depth of Field
        vec3 pos = ro + 1.5 * rd;
        ro.xy += vec2(rand(), rand()) * 0.01;
        rd = normalize(pos - ro);

        for (int j = 0; j < 5; j++)
        {
            float maxd = 50.0, t = 0.0;

            // raymarching algorithm
            for (int i = 0; i < 50; i++)
            {
                float h = menger(pos=(ro+rd*t));
                t += h;
                if (t > maxd || abs(h) < eps.x) break;
            }
    
            // we hit sky
            if (t >= maxd)
            {
                color += weight + vec3(-0.5, 0.0, 0.5) * rd.y;
                break;
            }

            // calculate normal vector (position derivative)
            vec3 normal = normalize(vec3(menger(pos + eps.xyy) - menger(pos - eps.xyy),
                                    menger(pos + eps.yxy) - menger(pos - eps.yxy),
                                    menger(pos + eps.yyx) - menger(pos - eps.yyx)));

            ro = pos + 0.001 * normal; // secondary ray origin
            rd = normalize(vec3(rand(), rand(), rand()) * 2.0 - 1.0);
            rd = dot(rd, normal) < 0.0 ? -rd : rd;
            weight *= dot(rd, normal);
        }
    }

    // gamma correction
    gl_FragColor = sqrt(color.xyzz / 100.0);
}