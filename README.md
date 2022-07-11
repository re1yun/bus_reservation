# bus_reservation
(1) 서버 기능: (bus_serv.c)
- 쓰레드를 이용하여 여러 개의 클라이언트 접속을 허용하지만, 결과는 요청한 클라이언트에게만
전송함
- 동기화 기법을 반드시 사용해야 됨
. 버스 좌석 예약 및 예약 취소에 대해 동기화 기법 사용
. 여러 클라이언트가 동시에 같은 좌석을 예약할 경우, 하나의 클라이언트만 예약 가능
- 예약 기능 구현 및 예약 현황을 요청한 클라이언트에게 전송함
- 예약 취소 기능 구현 및 다양한 예외 처리
. 예약을 하지 않은 클라이언트가 예약을 취소하는 경우, 예약 취소가 되면 안됨(return -4)
. 예약이 되지 않은 좌석을 취소하는 경우 예약 취소 실패 리턴(return -3)
(2) 클라이언트 기능: (bus_clnt.c)
- 송신, 수신 기능은 각각 별도의 쓰레드를 이용해서 구현
- 동기화 기법 사용: 송신, 수신 함수 내부
- 메뉴 (4개 메뉴로 구성됨): 1: 조회, 2: 예약, 3: 취소, 4: 종료
1) 조회 메뉴
- 전체 좌석 예약 현황 조회 메뉴 (서버에 요청하고 그 결과값을 화면에 출력)
2) 예약 메뉴
- 1회에 1자리만 가능, 좌석 번호를 입력 받아 해당 좌석을 예약함
- 이미 예약된 좌석은 예약이 불가능함
- 예약 현황을 화면에 출력함
3) 취소 메뉴
- 해당 좌석 번호를 입력 받아서 예약을 취소함
- 본인이 예약한 좌석만 취소가 가능해야 됨
- 취소 현황을 화면에 출력함
4) 종료 메뉴
- 소켓 연결 종료 기능 수행
5) 서버의 리턴값 출력
- 서버에서 전송한 각 명령에 대한 결과값을 화면에 출력함
