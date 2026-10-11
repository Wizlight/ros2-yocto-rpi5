import rclpy
from rclpy.node import Node

from std_msgs.msg import Int32
from std_srvs.srv import SetBool


class CounterPublisher(Node):
    def __init__(self):
        super().__init__('counter_publisher')

        self.publisher = self.create_publisher(
            Int32,
            'counter',
            10,
        )

        self.counter = 1
        self.enabled = True

        self.timer = self.create_timer(
            1.0,
            self.publish_counter,
        )

        self.service = self.create_service(
            SetBool,
            'counter/set_enabled',
            self.set_enabled,
        )

    def publish_counter(self):
        if not self.enabled:
            return

        message = Int32()
        message.data = self.counter

        self.publisher.publish(message)

        self.get_logger().info(
            f'Publishing: {message.data}'
        )

        self.counter += 1

    def set_enabled(self, request, response):
        self.enabled = request.data

        response.success = True

        if self.enabled:
            response.message = 'Counter started'
        else:
            response.message = 'Counter stopped'

        self.get_logger().info(response.message)

        return response


def main(args=None):
    rclpy.init(args=args)

    node = CounterPublisher()
    rclpy.spin(node)

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()