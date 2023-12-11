export VULTR_API_KEY='CF4BWYKTTEDOJHIEFCAQJB57VCWJPZWRXCEA'

function create_instance()
{
	curl "https://api.vultr.com/v2/instances" \
	  -X POST \
	  -H "Authorization: Bearer ${VULTR_API_KEY}" \
	  -H "Content-Type: application/json" \
	  --data '{
		"region" : "icn",
		"plan" : "vc2-1c-1gb",
		"label" : "Example Instance",
		"os_id" : 477,
		"backups" : "disabled",
		"hostname": "gptvpn",
		"sshkey_id":["c3096c8a-9a34-47b6-8a4b-10e5567c2259","8d5001c4-eabe-40f0-9dcb-577deadb56c7"],
		"script_id": "35887d27-d58a-47a8-9176-57bf5a951706",
		"tags": [
		  "a tag",
		  "another"
		]
	  }'
}

# get instance id, assume we got only one instance
function list_instance()
{
	# keep the output to a file
	curl "https://api.vultr.com/v2/instances" \
		-X GET \
		-H "Authorization: Bearer ${VULTR_API_KEY}" | tee > output.json

	# using shell script without any tool to parse the json
	# get the key `id` value
	id=$(cat output.json | sed -e 's/[{}]/''/g' | awk -v k="id" '{n=split($0,a,","); for (i=1; i<=n; i++) {if (a[i] ~ /"id":/) {print a[i]}}}' | cut -d '"' -f6)

	# return id
	echo $id
}

function delete_instance()
{
	echo "Delete instance $id"
	curl "https://api.vultr.com/v2/instances/${id}" \
		-X DELETE \
		-H "Authorization: Bearer ${VULTR_API_KEY}"
}

print_usage()
{
	echo "Usage : $0 [-c: create instance]
		[-d: delete instance]
		[-l: list instance id]
		[-h]"
}

if [ $# -lt 1 ]
then
	print_usage $0
fi

while getopts "cdlh" flag; do
	case $flag in
		c)
			# we only maintain one instance currently
			id=$(list_instance)
			[ -z $id ] && create_instance
			;;
		d)
			id=$(list_instance)
			[ -z $id ] && { echo "No instance to be deleted"; exit;}
			delete_instance $id
			;;
		l)
			id=$(list_instance)
			echo "InstanceId: $id"
			;;

		h)
			print_usage $0
			;;
		*)
			print_usage $0
			;;
	esac
done
