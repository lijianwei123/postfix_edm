<?php
//error_reporting(0);

define("SERVER_IP", "10.0.0.31");
define("SERVER_PORT", 60907);

define("VERSION", 1);
define("CLIENT_CPP", 1);
define("CLIENT_PHP", 2);
class Module
{
    const SID_COMMAND			= 1;
	const SID_MSG 			    = 2;
	const SID_OTHER 			= 3;
}

//for SID_COMMAND 
class Command
{    
    
    /**
     * @desc  开始
     */
	public  static  $START              = 1; 
	/**
     * @desc  停止
     */
	public  static  $STOP			    = 2; 
	/**
     * @desc  暂停
     */
	public  static  $PAUSE			    = 4;
	/**
     * @desc  继续
     */
	public  static  $RESUME	            = 8;
	/**
     * @desc  调整发送速率
     */
    public  static  $ADJUST_RATE        = 16;
}

//for SID_MSG
class Msg
{
	const CID_MSG_DATA					    = 1;
	const CID_MSG_SERVER_STATUS_INFO 		= 2;
	const CID_MSG_ALL_CLIENT_STATUS_INFO    = 3;
}

//for SID_OTHER
class Other
{
    const CID_OTHER_HEARTBEAT               = 1;
    const CID_OTHER_RESPONSE                = 2;
    const CID_OTHER_REG_CLIENT_TYPE         = 3;
}


class BaseClientController extends BaseController
{
    
    private $_sock = NULL;
    
    protected $timeout = 10;
    
    public function init()
    {
        parent::init();
        if (!Yii::app()->request->isAjaxRequest) 
            header("content-type:text/html;charset=utf-8");
    }
    
    protected function getSock()
    {
        if (is_resource($this->_sock))
            return $this->_sock;
        
        $sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        //$sock = fsockopen(SERVER_IP, SERVER_PORT, $errno, $err, $this->timeout);
        if (!$sock) {
            throw new Exception(socket_strerror(socket_last_error($sock)));
        }
        
        socket_set_option($sock, SOL_SOCKET, SO_RCVTIMEO, array('sec' => $this->timeout, 'usec' => 0));
        socket_set_option($sock, SOL_SOCKET, SO_SNDTIMEO, array('sec' => $this->timeout, 'usec' => 0));
        socket_set_option($sock, SOL_TCP, TCP_NODELAY, 1);
        socket_set_block($sock);
        
        if (!socket_connect($sock, SERVER_IP, SERVER_PORT)) {
            throw new Exception(socket_strerror(socket_last_error($sock)));
        }

        //stream_set_timeout($sock, $this->timeout);
        //socket_set_option($sock, TCP, TCP_NODELAY, 1);
        
        $this->_sock = $sock;
        if (is_resource($this->_sock))
            $this->regClientType();
        return $this->_sock;
    }
    
    //注册客户端类型
    protected function regClientType()
    {    
        $data_args = array('module_id' => Module::SID_OTHER, 'command_id' => Other::CID_OTHER_REG_CLIENT_TYPE, CLIENT_PHP);
        $bin_str = $this->pack($data_args, 'n');
        socket_send($this->_sock, $bin_str, strlen($bin_str), 0);
    }
    
    /**
     * @desc pack
     * @param array $data_args
     * @param string $data_format
     * @return string 二进制数据
     */
    protected function pack(array $data_args = array(), $data_format = "")
    {
        if (!$data_args) 
            throw new Exception("pack args error");
            
        $header_format = "Nnnnn";
        $header_pack_args = array(
            'pdu_len'		=> 0,
            'module_id'     => Module::SID_OTHER,
            'command_id'    => 0,
            'version_id'	=> VERSION,
            'reserve_id'	=> 0, 
        );
        
        $format = $header_format. $data_format;
        $pack_args = array_merge($header_pack_args, $data_args);        
        array_unshift($pack_args, $format);
        
        $pack_str = call_user_func_array("pack", $pack_args);
        $pdu_len = strlen($pack_str);
        $pack_args['pdu_len'] = $pdu_len;
        
        return call_user_func_array("pack", $pack_args);
    }
    
    /**
     * @desc unpack
     * @param string $data_format   Ndata_len/H{$data_hex_len}data
     * @param string $bin_str
     * @return array 
     */
    protected function unpack($data_format, $bin_str)
    {
        $header_unpack_format = "Npdu_len/nmodule_id/ncommand_id/nversion_id/nreserve_id/";
        
        return unpack($header_unpack_format. $data_format, $bin_str);
    }
    
    protected function send($bin_str = '')
    {
       return  socket_send($this->sock, $bin_str, strlen($bin_str), 0);
    }
    
    protected function recv()
    {
        return socket_read($this->sock, 1024);
    }
    
    
    public function __destruct()
    {
        if (is_resource($this->_sock))
            socket_close($this->_sock);
    }
    
    protected function getDocComment($str, $tag = '')
    {
        if (empty($tag)) {
            return $str;
        }

        $matches = array();
        preg_match("/".$tag."(.*)(\\r\\n|\\r|\\n)/U", $str, $matches);

        if (isset($matches[1])) {
            return trim($matches[1]);
        } 

        return ''; 
    }
}

/**
 * @desc      EDM管理
 * @author    lijianwei    2014-11-11
 */
class EdmController extends BaseClientController
{
    public function init()
    {
        parent::init();
    }
    
    public function actionIndex()
    {
        $reflect = new ReflectionClass("Command");
        $commands = $reflect->getStaticProperties();
       
        $commandInfos = array();
        foreach ($commands as $commandName => $commandValue) {
            $reflectProperty = new ReflectionProperty("command", $commandName);
            $commandInfo['name'] = $commandName;
            $commandInfo['value'] = $commandValue;
            $commandInfo['human'] = $this->getDocComment($reflectProperty->getDocComment(), 'desc');
            $commandInfo['url'] = $this->createUrl('postfix/edm/servercommand', array('command' => $commandValue));
            $commandInfos[$commandValue] = $commandInfo;
        }
        Yii::app()->smarty->assign("commandInfos", $commandInfos);
          

        //获取一些服务器状态
        $serverStatusInfo = $this->getServerStatusInfo();
        $this->batchAssign($serverStatusInfo);
        
        $this->smartyRender("postfix/index.html");
    }
    

    public function actionServerCommand()
    {
        $command = intval(Yii::app()->request->getParam("command"));
 
        $reflect = new ReflectionClass("Command");
        $commands = $reflect->getStaticProperties();
      
        if (!in_array($command, $commands)) {
            $this->ajaxError(-1, "你想干什么？");
        }
      
  
        //发送给服务端
        $data_args = array('module_id' => Module::SID_COMMAND, 'command_id' => $command, $command);
        $data_format = 'n';
        $bin_str = $this->pack($data_args, $data_format);
        
        try {
            $send_len = $this->send($bin_str);
            if (!$send_len) {
                $this->ajaxError(-1, "发送数据失败");
            }
            $recv_bin = $this->recv();
            if (!$recv_bin) {
                $this->ajaxError(-1, "服务器是不是感冒了");
            }
            
            $data = $this->unpack("nstatus/Nmsg_len/A*", $recv_bin);
            if (!$this->isServerResponsePacket($data)) {
                $this->ajaxError(-1, "服务器神经错乱了");
            }
            if (isset($data['status']) && $data['status'] == 0) {
                $this->ajaxOutput();
            } else {
                $this->ajaxError($data['status'], array_pop($data));
            }
        } catch (Exception $e) {
            $this->ajaxError(-1, $e->getMessage()); 
        }
    }
    
    //调整发送速率
    public function actionAdjustRate()
    {
        $delay = intval(Yii::app()->request->getPost('delay'));
        $multi = intval(Yii::app()->request->getPost('multi'));
        
        $data_args = array('module_id' => Module::SID_COMMAND, 'command_id' => Command::$ADJUST_RATE, $delay, $multi);
        $data_format = "nn";
        $bin_str = $this->pack($data_args, $data_format);
        
        try {
            $send_len = $this->send($bin_str);
            if (!$send_len) {
                $this->ajaxError(-1, "发送数据失败");
            }
            $recv_bin = $this->recv();
            if (!$recv_bin) {
                $this->ajaxError(-1, "服务器是不是感冒了");
            }
            
            $data = $this->unpack("nstatus/Nmsg_len/A*", $recv_bin);
            if (!$this->isServerResponsePacket($data)) {
                $this->ajaxError(-1, "服务器神经错乱了");
            }
            
            if (isset($data['status']) && $data['status'] == 0) {
                $this->ajaxOutput();
            } else {
                $this->ajaxError($data['status'], array_pop($data));
            }
        } catch (Exception $e) {
            $this->ajaxError(-1, $e->getMessage()); 
        }
    }
    
    //获取所有客户端状态
    public function actionGetAllClientStatus()
    {
        $all_client_status = array(
            '10.0.0.231' => array(
                'ip' => '10.0.0.231',
                'emailFromAddr' => 'no_reply@xz.9first.com',
            ),
            
            '10.0.0.232' => array(
                'ip' => '10.0.0.232',
                'emailFromAddr' => 'no_reply@xz.9first.com',
            ),
            
           '10.0.0.233' => array(
                'ip' => '10.0.0.233',
                'emailFromAddr' => 'no_reply@xz.9first.com',
            ),
            
           '10.0.0.234' => array(
                'ip' => '10.0.0.234',
                'emailFromAddr' => 'no_reply@xz.9first.com',
            ),
            
           '10.0.0.235' => array(
                'ip' => '10.0.0.235',
                'emailFromAddr' => 'no_reply@xz.9first.com',
            ),
        );
        
        //$this->ajaxOutput(compact("all_client_status"));
        
        //发送给服务端
        $data_args = array('module_id' => Module::SID_MSG, 'command_id' => Msg::CID_MSG_ALL_CLIENT_STATUS_INFO);
        $bin_str = $this->pack($data_args);
        try {
            $send_len = $this->send($bin_str);
            if (!$send_len) {
                $this->ajaxError(-1, "发送数据失败");
            }
            $recv_bin = $this->recv();
            if (!$recv_bin) {
                $this->ajaxError(-1, "服务器是不是感冒了");
            }
            
            $data = $this->unpack("Ndata_len/A*", $recv_bin);
            if (!$data['data_len']) {
                $this->ajaxError(-1, "孩子还 没有出生");
            } else {
                $all_client_status = json_decode(array_pop($data));
                $this->ajaxOutput(compact("all_client_status"));
            }
        } catch (Exception $e) {
            $this->ajaxError(-1, $e->getMessage()); 
        }
    }
    
    //test  
    public function actionTest()
    {
        //发送给服务端
        $data_args = array('module_id' => Module::SID_MSG, 'command_id' => Msg::CID_MSG_DATA, strlen("hello world"), "hello world");
        $data_format = 'NA*';
        $bin_str = $this->pack($data_args, $data_format);
        $this->send($bin_str);
        echo bin2hex($bin_str);
        echo "<br>";
        
        $recv_bin = $this->recv();
        echo bin2hex($recv_bin);exit;
        
    }
    
    //验证是否服务器返回的数据包
    protected function isServerResponsePacket(array $data)
    {
        return $data['module_id'] == Module::SID_OTHER  && $data['command_id'] == OTHER::CID_OTHER_RESPONSE;
    }
    
    protected function isServerStatusPacket(array $data)
    {
        return $data['module_id'] == Module::SID_MSG && $data['command_id'] == MSG::CID_MSG_SERVER_STATUS_INFO;
    }
    
    
    protected function  getServerStatusInfo()
    {   
        //发送状态请求
        $data_args = array('module_id' => Module::SID_MSG, 'command_id' => MSG::CID_MSG_SERVER_STATUS_INFO);
        $bin_str = $this->pack($data_args);
     
        $send_len = $this->send($bin_str);
        if (!$send_len) {
            return array();
        }
        
        $recv_bin = $this->recv();
        if (!$recv_bin) {
            return array();
        }
        $data = $this->unpack("Ndata_len/A*", $recv_bin);
        if ($this->isServerStatusPacket($data)) {
            return json_decode(array_pop($data));
        }
        return array(); 
           
    }
}