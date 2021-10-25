
### 数据集的构建：[使用传统方法完成3D稀疏重建](https://zhuanlan.zhihu.com/p/184978050)

- 下载第三方库并安装（见3rd-party文件目录，[安装教程](https://colmap.github.io/install.html)
- 创建数据集文件夹, 注意多拍摄不同角度的图片，并且图片分辨率需要保持一致
- 如下转换成LLFF格式

```bash
git clone https://hub.fastgit.org/Fyusion/LLFF.git
python img2poses.py /root/nerf-pytorch/data/4paradigm/coffee 
```

### 运行NeRF 

- 添加配置文件，并运行。最后合成的动态效果见log目录
```
python run_nerf.py --config configs/4paradigm/coffee.txt
```


### poses_bounds.npy 文件格式

N x 17, 其中N代表输入图像的数目
3 x 5 pose matri:
    3 x 4 camera-to-world affine transform 
    3 x 1 : [height, width, focal_length], we assume the the principal point is centered and 
        that the focal length is the same for both x and y 
2 depth values:
    bound the closest and farthest scene content from the point of view


### 如何渲染路径 

- python run_nerf.py --config configs/4paradigm/coffee.txt --render_only --render_test 

``render_path``
Args:
    - render_poses  
    - hwf   图像高度、宽度和焦距
    - K 
    - chunk 切块，用于节约内存 
    - render_kwargs
    - gt_imgs 
    - savedir 
    - render_factor 


- Camera-to-world transformation matrix: 3 x 4 



### 如何可视化

- python visualization_nerf.py --config configs/4paradigm/doll.txt --render_only


