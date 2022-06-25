#### 安装源
- 1.添加GPG Key:`curl -fsSL http://mirrors.aliyun.com/kubernetes/apt/doc/apt-key.gpg | sudo apt-key add -`
- 2.添加kubernetes软件源:`sudo add-apt-repository "deb http://mirrors.aliyun.com/kubernetes/apt kubernetes-xenial main`

#### 三大工具组件
- 1.`kubeadm`:引导启动Kubernate集群的命令行工具。
- 2.`kubelet`:在集群中所有计算机上运行的组件，用来执行如启动Pods和Containers等操作。
- 3.`kubectl`:用于操作运行中集群的命令行工具。

#### 镜像安装
- 1.需要先安装docker，以docker为基石构建集群。`sudo apt get install docker.io`
- 2.获取当前版本所需要镜像 `kubeadm config images list --kubernetes-version v1.24.2` 其中version版本以kubelet为准。
- 2.预拉取镜像脚本
	```
	# 拉取镜像
	sudo docker pull k8simage/kube-apiserver:v1.24.2
	sudo docker pull k8simage/kube-controller-manager:v1.24.2
	sudo docker pull k8simage/kube-scheduler:v1.24.2
	sudo docker pull k8simage/kube-proxy:v1.24.2
	sudo docker pull k8simage/pause:3.7
	sudo docker pull k8simage/etcd:3.5.3-0
	sudo docker pull k8simage/coredns:v1.8.6

	# 修改镜像标签
	sudo docker tag k8simage/kube-apiserver:v1.24.2 k8s.gcr.io/kube-apiserver:v1.24.2
	sudo docker tag k8simage/kube-controller-manager:v1.24.2 k8s.gcr.io/kube-controller-manager:v1.24.2
	sudo docker tag k8simage/kube-scheduler:v1.24.2 k8s.gcr.io/kube-scheduler:v1.24.2
	sudo docker tag k8simage/kube-proxy:v1.24.2 k8s.gcr.io/kube-proxy:v1.24.2
	sudo docker tag k8simage/pause:3.7 k8s.gcr.io/pause:3.7
	sudo docker tag k8simage/etcd:3.5.3-0 k8s.gcr.io/etcd:3.5.3
	sudo docker tag k8simage/coredns:v1.8.6 k8s.gcr.io/coredns/coredns:v1.8.6

	# 删除镜像
	sudo docker rmi k8simage/kube-controller-manager:v1.24.2
	sudo docker rmi k8simage/kube-scheduler:v1.24.2
	sudo docker rmi k8simage/kube-proxy:v1.24.2
	sudo docker rmi k8simage/pause:3.7
	sudo docker rmi k8simage/etcd:3.5.3-0
	sudo docker rmi k8simage/coredns:v1.8.6
	```

#### 初始化集群控制
- 1.确保kubelet开机启动:`sudo systemctl start kubelet | sudo systemctl enable kubelet`
- 2.初始化主集群，需要su权限:`kubeadm init --kubernetes-version v1.24.2 --pod-network-cidr=10.244.0.0/16`
- 3.安装flannel:`curl --insecure -sfL https://raw.githubusercontent.com/coreos/flannel/master/Documentation/kube-flannel.yml | kubectl apply -f -`
- 4.移除master污点:`kubectl taint nodes --all node-role.kubernetes.io/master-`
- 5.调整NodePort范围:(1)在etc/kubernetes/manifests/kube-apiserver.yaml中的spec.containers.command加入参数`--service-node-port-range=1-32767` (2)重启Api-Server:1)获得apiserver-pod的名字:`export apiserver_pods=$(kubectl get pods --selector=component=kube-apiserver -n kube-system --output=jsonpath={.items..metadata.name})` 2)删除Api-Server的pod:`kubectl delete pod $apiserver_pods -n kube-system` 3)验证修改结果，确认参数存在:`kubectl delete pod $apiserver_pods -n kube-system`
- 6.启用kubectl命令自动补全功能:(1)`kubectl delete pod $apiserver_pods -n kube-system` (2):`echo 'source /usr/share/bash-completion/bash_completion' >> ~/.bashrc`
- 7.打印join命令:`kubeadm token create --print-join-command`
#### 使用clash梯子

	```
	# first
	sudo systemctl set-environment HTTP_PROXY=127.0.0.1:7890
	sudo systemctl set-environment HTTPS_PROXY=127.0.0.1:7890
	sudo systemctl restart containerd.service
	# then
	sudo kubeadm config images pull
	```