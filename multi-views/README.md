
requirements:
- ImageMagick 


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