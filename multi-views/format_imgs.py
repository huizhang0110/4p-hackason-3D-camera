import cv2 
import os 

img_folder = "/root/nerf-pytorch/data/4paradigm/coffee/_origin"
save_folder = os.path.join(os.path.dirname(img_folder), "images")
if not os.path.exists(save_folder): os.makedirs(save_folder)


format_size = None 
for i, img_fpath in enumerate(os.listdir(img_folder)):
    img_fpath = os.path.join(img_folder, img_fpath)
    img = cv2.imread(img_fpath)
    # if format_size is None:
    #     height, width = img.shape[:2]
    #     format_size = (width, height)
    # else:
    #     img = cv2.resize(img, format_size)
    #     print(format_size)
    cv2.imwrite(os.path.join(save_folder, "%d.jpg" % i), img)
