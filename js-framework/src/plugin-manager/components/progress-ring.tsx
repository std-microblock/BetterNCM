export const ProgressRing: React.FC<{
	size?: string;
}> = (props) => {
	return (
		<span
			className="bncm-spinner"
			style={{
				width: props.size || "16px",
				height: props.size || "16px",
			}}
		>
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
		</span>
	);
};
